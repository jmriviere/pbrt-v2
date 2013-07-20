/*
 * gpurenderer.cpp
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#include <iostream>

#include "scene.h"
#include "film.h"
#include "sampler.h"
#include "progressreporter.h"
#include "camera.h"
#include "intersection.h"
#include "renderers/gpurenderer.h"
#include "host.h"
#include "util.h"
#include "accelerators/bvh.h"
#include "imageio.h"
#include <time.h>

static LoggerPtr logger(Logger::getLogger(__FILE__));

// SamplerRenderer Method Definitions
GpuRenderer::GpuRenderer(std::vector<Light*> lights, std::vector<Reference<Shape> > primitives,
		Sampler *s, Camera *c, bool visIds)  {
	BasicConfigurator::configure();
	camera = c;
	visualizeObjectIds = visIds;

	Metadata* meta;
	float* data;

	uint32_t offset = 0;

	for (std::vector<Reference<Shape> >::iterator it = primitives.begin();
			it != primitives.end(); ++it) {
		meta = new Metadata;
		uint32_t c = (*it)->toGPU(meta, NULL);
		data = new float[c];
		c = (*it)->toGPU(meta, data);
		meta->offset = offset;
		offset += c;
		this->meta_primitives.push_back(*meta);
		this->primitives.push_back(*data);
		delete meta;
	}
}


GpuRenderer::~GpuRenderer() {
	delete camera;
}

void GpuRenderer::Render(const Scene *scene) {
	PBRT_FINISHED_PARSING();
	PBRT_STARTED_RENDERING();

	cl_int err;
	Metadata meta;

	GPUCamera gpuC = camera->toGPU();

	Light* map = scene->lights[0];

	size_t c = map->toGPU(&meta, NULL);


	float* env = new float[c];
	map->toGPU(&meta, env);

	float env_w = meta.dim[0];
	float env_h = meta.dim[1];

	this->meta_primitives.push_back(meta);

	uint32_t nPixels = camera->film->xResolution * camera->film->yResolution;

	cl::Event ev;

	float *Ls = new float[4 * nPixels];

	Host::instance().buildKernels(KERNEL_PATH);

	cl::Kernel k = Host::instance().retrieveKernel("ray_cast");

	cl::Image2D envgpu(*(Host::instance()._context), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			cl::ImageFormat(CL_RGBA, CL_FLOAT), env_w, env_h, 0, env, &err);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error creating the image from the envmap: " << err
				<< " at line " << __LINE__);
	}

	cl::size_t<3> origin;
	origin[0] = 0;
	origin[1] = 0;
	origin[2] = 0;

	cl::size_t<3> region;
	region[0] = env_w;
	region[1] = env_h;
	region[2] = 1;

	err = Host::instance()._queue->enqueueWriteImage(envgpu, CL_TRUE, origin, region, 0, 0,
			env, NULL, NULL);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error writing the envmap to device memory: " << err
				<< " at line " << __LINE__);
	}

	cl::Buffer bufLs(*(Host::instance())._context, CL_MEM_WRITE_ONLY,
			4 * nPixels * sizeof(float), NULL, &err);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error creating the output buffer: " << err << " at line " << __LINE__);
	}

	cl::Buffer bufCam(*(Host::instance())._context, CL_MEM_READ_ONLY,
			sizeof(GPUCamera), NULL, &err);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error creating the camera buffer: " << err << " at line " << __LINE__);
	}

	err = Host::instance()._queue->enqueueWriteBuffer(bufCam, CL_TRUE, 0,
			sizeof(GPUCamera), &gpuC, NULL, NULL);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error writing the camera buffer to device memory: "
				<< err << " at line " << __LINE__);
	}

	cl::Buffer buf_prims(*(Host::instance())._context, CL_MEM_READ_ONLY,
			primitives.size() * sizeof(GPUSphere), NULL, &err);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error creating the raw data buffer: "
				<< err << " at line " << __LINE__);
	}

	err = Host::instance()._queue->enqueueWriteBuffer(buf_prims, CL_TRUE, 0,
			primitives.size() * sizeof(GPUSphere),
			&primitives[0], NULL, NULL);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error writing the raw data buffer to device memory: "
				<< err << " at line " << __LINE__);
	}

	cl::Buffer buf_mprims(*(Host::instance())._context, CL_MEM_READ_ONLY,
			meta_primitives.size() * sizeof(Metadata), NULL, &err);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error creating the metadata buffer: "
				<< err << " at line " << __LINE__);
	}

	err = Host::instance()._queue->enqueueWriteBuffer(buf_mprims, CL_TRUE, 0,
			meta_primitives.size() * sizeof(Metadata),
			&meta_primitives[0], NULL, NULL);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error writing the metadata buffer to device memory: "
				<< err << " at line " << __LINE__);
	}

	k.setArg(0, bufLs);
	k.setArg(1, bufCam);
	k.setArg(2, 1);
	k.setArg(3, (uint32_t)meta_primitives.size());
	k.setArg(4, buf_mprims);
	k.setArg(5, buf_prims);
	k.setArg(6, envgpu);

	err = Host::instance()._queue->enqueueNDRangeKernel(k, cl::NullRange,
			cl::NDRange(camera->film->xResolution, camera->film->yResolution),
			cl::NullRange, NULL, &ev);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error calling the kernel: "
				<< err << " at line " << __LINE__);
	}

	ev.wait();

	err = Host::instance()._queue->enqueueReadBuffer(bufLs, CL_TRUE, 0,
			4 * nPixels * sizeof(float), Ls, NULL, NULL);

	if (CL_SUCCESS != err) {
		LOG(logger, ERROR, "Error reading back from device: "
				<< err << " at line " << __LINE__);
	}

	float v[3];

	int j = 0;

	for (uint32_t y = 0; y < camera->film->yResolution; ++y) {
		for (uint32_t x = 0; x < camera->film->xResolution; ++x) {
			v[0] = Ls[j];
			v[1] = Ls[j + 1];
			v[2] = Ls[j + 2];
			j += 4;

			CameraSample s;
			s.imageX = x;
			s.imageY = y;

			camera->film->AddSample(s, Spectrum::FromRGB(v));
		}
	}

	camera->film->WriteImage();

	delete Ls;
	delete env;
}

Spectrum GpuRenderer::Li(const Scene* scene, const RayDifferential & ray, const Sample* sample,
		RNG& rng, MemoryArena & arena, Intersection *isect, Spectrum* T) const {
	return Spectrum(1.0f);
}

Spectrum GpuRenderer::Transmittance(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena) const {
	return Spectrum(1.0f);
}

/*GpuRenderer* CreateGpuRenderer(const ParamSet &params, Camera *camera) {

}*/
