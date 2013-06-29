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

// SamplerRenderer Method Definitions
GpuRenderer::GpuRenderer(std::vector<Reference<Shape> > primitives, Sampler *s, Camera *c, bool visIds)  {
	BasicConfigurator::configure();
    sampler = s;
    camera = c;
    visualizeObjectIds = visIds;
    GPUSphere* shape;

    for (std::vector<Reference<Shape> >::iterator it = primitives.begin();
    	 it != primitives.end(); ++it) {
    	size_t c = (*it)->toGPU(NULL);
    	shape = (GPUSphere*)malloc(c);
    	c = (*it)->toGPU(shape);
    	this->primitives.push_back(*shape);
    }
}


GpuRenderer::~GpuRenderer() {
    delete sampler;
    delete camera;
}

void GpuRenderer::Render(const Scene *scene) {
/*    PBRT_FINISHED_PARSING();
    PBRT_STARTED_RENDERING();*/

	int env_w, env_h;

	// TODO: Get the envmap from the scene.
	RGBSpectrum* envmap = ReadImage("/homes/jmr12/Thesis/pbrt-v2/scenes/textures/grace_latlong.exr",
			&env_w, &env_h);

	float* env = new float[env_w * env_h * 4];

	for (int i = env_w * env_h; i > 0 ; --i) {
		float rgb[4];
		envmap[i].ToRGB(rgb);
		rgb[3] = 1.0;
		std::memcpy(&env[4 * (env_w * env_h - i)], rgb, 4 * sizeof(float));
	}

	std::vector<std::pair<size_t, Sample*> > samples;
	std::vector<gpu_Ray> rays = generateGpuRays(sampler, scene, camera, samples);

	uint32_t nRays = rays.size();

	cl_int kepasa;

    uint32_t nPixels = camera->film->xResolution * camera->film->yResolution;

    cl::Event ev;

    float *Ls = new float[4 * nRays];

    std::cout << env_w << " " << env_h << std::endl;

    Host::instance().buildKernels(KERNEL_PATH);

    cl::Kernel k = Host::instance().retrieveKernel("ray_cast");

    cl::Image2D envgpu(*(Host::instance()._context), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_RGBA, CL_FLOAT),
    				   env_w, env_h, 0, env, &kepasa);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrIm " << kepasa << std::endl;
    }

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    cl::size_t<3> region;
    region[0] = env_w;
    region[1] = env_h;
    region[2] = 1;

    kepasa = Host::instance()._queue->enqueueWriteImage(envgpu, CL_TRUE, origin, region, 0, 0, env, NULL, NULL);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrImW " << kepasa << std::endl;
    }

    cl::Buffer bufLs(*(Host::instance())._context, CL_MEM_WRITE_ONLY, 4 * nRays * sizeof(float), NULL, &kepasa);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrBufLs " << kepasa << std::endl;
    }

    cl::Buffer buf_rays(*(Host::instance())._context, CL_MEM_READ_ONLY, nPixels * sampler->samplesPerPixel * sizeof(gpu_Ray), NULL, &kepasa);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrBufR " << kepasa << std::endl;
    }

    cl::Buffer buf_prims(*(Host::instance())._context, CL_MEM_READ_ONLY, primitives.size() * sizeof(GPUSphere), NULL, &kepasa);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrBuf " << kepasa << std::endl;
    }

    kepasa = Host::instance()._queue->enqueueWriteBuffer(buf_prims, CL_TRUE, 0, primitives.size() * sizeof(GPUSphere), &primitives[0], NULL, NULL);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrWrite " << kepasa << std::endl;
    }

    kepasa = Host::instance()._queue->enqueueWriteBuffer(buf_rays, CL_TRUE, 0, nPixels * sampler->samplesPerPixel * sizeof(gpu_Ray), &rays[0], NULL, NULL);


    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrWriteR " << kepasa << std::endl;
    }


    k.setArg(0, envgpu);
    k.setArg(1, bufLs);
    k.setArg(2, buf_rays);
    k.setArg(3, (uint32_t)primitives.size());
    k.setArg(4, buf_prims);


    kepasa = Host::instance()._queue->enqueueNDRangeKernel(k, cl::NullRange, cl::NDRange(nRays),
    											  cl::NDRange(1), NULL, &ev);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrND " << kepasa << std::endl;
    }

    ev.wait();

    kepasa = Host::instance()._queue->enqueueReadBuffer(bufLs, CL_TRUE, 0, 4 * nRays * sizeof(float), Ls, NULL, NULL);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrRead " << kepasa << std::endl;
    }

    std::cout << "Number of rays generated: " << nRays << std::endl;

    float v[3];

    srand(time(NULL));

    int j = 0;

    for (vector<std::pair<size_t, Sample*> >::iterator it = samples.begin();
    	 it != samples.end(); ++it) {
    	for (size_t i = 0; i < it->first; ++i) {
    		v[0] = Ls[j];
    		v[1] = Ls[j + 1];
    		v[2] = Ls[j + 2];
			//std::cout << Ls[j] << " " << Ls[j+1] << " " << Ls[j+2] << std::endl;
    		j += 4;
    		//std::cout << it->second[i].imageX << " " << it->second[i].imageY << std::endl;
    		camera->film->AddSample(it->second[i], Spectrum::FromRGB(v));
    	}
    }
    //camera->film->UpdateDisplay(sampler->xPixelStart,
    //        sampler->yPixelStart, sampler->xPixelEnd+1, sampler->yPixelEnd+1);
    camera->film->WriteImage();
    std::cout << sizeof(GPUSphere) << std::endl;
}

Spectrum GpuRenderer::Li(const Scene* scene, const RayDifferential & ray, const Sample* sample,
						 RNG& rng, MemoryArena & arena, Intersection *isect, Spectrum* T) const {
	return Spectrum(1.0f);
}

Spectrum GpuRenderer::Transmittance(const Scene *scene, const RayDifferential &ray,
					   const Sample *sample, RNG &rng, MemoryArena &arena) const {
	return Spectrum(1.0f);
}

static std::vector<gpu_Ray> generateGpuRays(Sampler* sampler, const Scene* scene,
											   const Camera* camera, std::vector<std::pair<size_t, Sample*> >& samples) {
    RNG rng(time(NULL));

    uint32_t maxSamples = sampler->MaximumSampleCount();
    Sample* origSample = new Sample(sampler, NULL, NULL, scene);
    Sample* sampleBuf = origSample->Duplicate(maxSamples);

    Ray *rays = new Ray[maxSamples];
    vector<gpu_Ray> raysBuf;

    uint32_t sampleCount;

    while (sampleCount = sampler->GetMoreSamples(sampleBuf, rng)) {
    	Sample* tmp =  origSample->Duplicate(maxSamples);
    	for (uint32_t i = 0; i < sampleCount; ++i) {
    		float rayWeight = camera->GenerateRay(sampleBuf[i], &rays[i]);
    		tmp[i] = sampleBuf[i];
    		gpu_Ray* r = new gpu_Ray;
    		for (uint32_t j = 0; j < 3; ++j) {
    			r->direction[j] = rays[i].d[j];
    			r->origin[j] = rays[i].o[j];
    		}
    		raysBuf.push_back(*r);
    	}
    	samples.push_back(std::make_pair(sampleCount, tmp));
    }

    return raysBuf;
}

/*GpuRenderer* CreateGpuRenderer(const ParamSet &params, Camera *camera) {

}*/
