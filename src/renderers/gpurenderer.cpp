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
#include <time.h>

#pragma pack(push, 1)

typedef struct s_ray {
	float origin[3];
	float dummy;
	float direction[3];
	float ultradummy;
} g_Ray;

typedef struct s_sphere {
	float center[3];
	float dummy;
	float radius;
} g_Sphere;

#pragma pack(pop)

// SamplerRenderer Method Definitions
GpuRenderer::GpuRenderer(std::vector<Reference<Shape> > primitives, Sampler *s, Camera *c, bool visIds) : primitives(primitives) {
	BasicConfigurator::configure();
    sampler = s;
    camera = c;
    visualizeObjectIds = visIds;
}


GpuRenderer::~GpuRenderer() {
    delete sampler;
    delete camera;
}

void GpuRenderer::Render(const Scene *scene) {
/*    PBRT_FINISHED_PARSING();
    PBRT_STARTED_RENDERING();*/

	cl_int kepasa;

    RNG rng(0);

    uint32_t nPixels = camera->film->xResolution * camera->film->yResolution;

    uint32_t maxSamples = sampler->MaximumSampleCount();
    Sample* origSample = new Sample(sampler, NULL, NULL, scene);
    Sample* samples = origSample->Duplicate(maxSamples);

    Ray *rays = new Ray[maxSamples];
    vector<g_Ray> raysBuf;

    uint32_t sampleCount;

    uint32_t p_index = 0;

    vector<std::pair<int, Sample*> > aggregate;

    std::cout << "Start" << std::endl;

    std::cout << primitives.size();

    const Transform *t = primitives[0]->WorldToObject;

    Ray rw;

    while (sampleCount = sampler->GetMoreSamples(samples, rng)) {
    	Sample* test = origSample->Duplicate(sampleCount);
    	for (uint32_t i = 0; i < sampleCount; ++i) {
    		float rayWeight = camera->GenerateRay(samples[i], &rays[i]);
    		(*t)(rays[i], &rw);
    		test[i] = samples[i];
    		g_Ray* r = new g_Ray;
    		for (uint32_t j = 0; j < 3; ++j) {
    			r->direction[j] = rw.d[j];
    			r->origin[j] = rw.o[j];
    		}
    		raysBuf.push_back(*r);
    	}
    	aggregate.push_back(std::make_pair(sampleCount, test));
    	++p_index;
    }

    g_Sphere sph;
    sph.center[0] = 0;
    sph.center[1] = 0;
    sph.center[2] = 0;
    sph.radius = 0.5;

    g_Sphere prims[1] = {sph};

    cl::Event ev;

    float *Ls = new float[raysBuf.size()];

    Host::instance().buildKernels(KERNEL_PATH);

    cl::Kernel k = Host::instance().retrieveKernel("ray_cast");

    cl::Buffer bufLs(*(Host::instance())._context, CL_MEM_WRITE_ONLY, raysBuf.size() * sizeof(float), NULL, &kepasa);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrBufLs " << kepasa << std::endl;
    }

    cl::Buffer buf_rays(*(Host::instance())._context, CL_MEM_READ_ONLY, nPixels * sampler->samplesPerPixel * sizeof(g_Ray), NULL, &kepasa);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrBufR " << kepasa << std::endl;
    }

    cl::Buffer buf_prims(*(Host::instance())._context, CL_MEM_READ_ONLY, sizeof(g_Sphere), NULL, &kepasa);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrBuf " << kepasa << std::endl;
    }

    kepasa = Host::instance()._queue->enqueueWriteBuffer(buf_prims, CL_TRUE, 0, sizeof(g_Sphere), &prims[0], NULL, NULL);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrWrite " << kepasa << std::endl;
    }

    kepasa = Host::instance()._queue->enqueueWriteBuffer(buf_rays, CL_TRUE, 0, nPixels * sampler->samplesPerPixel * sizeof(g_Ray), &raysBuf[0], NULL, NULL);

/*    for (int i = 0; i < raysBuf.size(); i++) {
    	std::cout << "Ray n° " << i << std::endl
    			  << "Direction = {" << raysBuf[i].direction[0] << "," << raysBuf[i].direction[1] << "," << raysBuf[i].direction[2] << "}" << std::endl
    			  << "Origin = {" << raysBuf[i].origin[0] << "," << raysBuf[i].origin[1] << "," << raysBuf[i].origin[2] << "}" << std::endl;
    }*/

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrWriteR " << kepasa << std::endl;
    }


    k.setArg(0, bufLs);
    k.setArg(1, buf_rays);
    k.setArg(2, 1);
    k.setArg(3, buf_prims);


    kepasa = Host::instance()._queue->enqueueNDRangeKernel(k, cl::NullRange, cl::NDRange(raysBuf.size()),
    											  cl::NDRange(1), NULL, &ev);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrND " << kepasa << std::endl;
    }

    ev.wait();

    kepasa = Host::instance()._queue->enqueueReadBuffer(bufLs, CL_TRUE, 0, raysBuf.size() * sizeof(float), Ls, NULL, NULL);

    if (CL_SUCCESS != kepasa) {
    	std::cout << "ErrRead " << kepasa << std::endl;
    }

    std::cout << "Number of rays generated: " << raysBuf.size() << std::endl;

    float v[3] = {1, 0, 0};

    srand(time(NULL));

    int j = 0;

    for (vector<std::pair<int, Sample*> >::iterator it = aggregate.begin();
    	 it != aggregate.end(); ++it) {
    	for (int i = 0; i < it->first; ++i) {
    		v[0] = Ls[++j];
    		v[1] = 0.0;
    		v[2] = 0.0;
    		camera->film->AddSample(it->second[i], RGBSpectrum::FromRGB(v));
    	}
    }
    //camera->film->UpdateDisplay(sampler->xPixelStart,
    //        sampler->yPixelStart, sampler->xPixelEnd+1, sampler->yPixelEnd+1);
    camera->film->WriteImage();
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
