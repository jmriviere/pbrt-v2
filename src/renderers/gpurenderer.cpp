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

typedef struct s_ray {
	float origin[3];
	float direction[3];
} g_Ray;

typedef struct s_sphere {
	float center[3];
	float radius;
} g_Sphere;

// SamplerRenderer Method Definitions
GpuRenderer::GpuRenderer(Sampler *s, Camera *c, bool visIds) {
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

    RNG rng(0);

    uint32_t nPixels = camera->film->xResolution * camera->film->yResolution;

    uint32_t maxSamples = sampler->MaximumSampleCount();
    Sample* origSample = new Sample(sampler, NULL, NULL, scene);
    Sample* samples = origSample->Duplicate(maxSamples);

    Ray *rays = new Ray[maxSamples];
    vector<g_Ray*> raysBuf;

    uint32_t sampleCount;

    uint32_t p_index = 0;

    std::cout << "Start" << std::endl;

    while (sampleCount = sampler->GetMoreSamples(samples, rng)) {
    	for (uint32_t i = 0; i < sampleCount; ++i) {
    		float rayWeight = camera->GenerateRay(samples[i], &rays[i]);
    		g_Ray* r = new g_Ray;
    		for (uint32_t j = 0; j < 3; ++j) {
    			r->direction[j] = rays[i].d[j];
    			r->origin[j] = rays[i].d[j];
    		}
    		raysBuf.push_back(r);
    	}
    	++p_index;
    }

    g_Sphere sph;
    sph.center[0] = -1.25;
    sph.center[1] = 0;
    sph.center[2] = 0;
    sph.radius = 0.5;

    g_Sphere prims[1] = {sph};

    cl::Event ev;

    std::cout << "Lol wut" << std::endl;



    Host::instance().buildKernels(KERNEL_PATH);

    cl::Kernel k = Host::instance().retrieveKernel("ray_cast");

    cl::Image2D im(*(Host::instance()._context), CL_MEM_WRITE_ONLY, cl::ImageFormat(CL_RGB, CL_FLOAT),
    			camera->film->xResolution, camera->film->yResolution, 0, NULL, NULL);
    cl::Buffer buf_rays(CL_MEM_READ_ONLY, nPixels * sampler->samplesPerPixel, &raysBuf[0], NULL);
    cl::Buffer buf_prims(CL_MEM_READ_ONLY, 1, &prims[0], NULL);

    k.setArg(0, im);
    k.setArg(1, buf_rays);
    k.setArg(2, 1);
    k.setArg(3, buf_prims);

    Host::instance()._queue->enqueueNDRangeKernel(k, cl::NullRange, cl::NDRange(800, 400),
    											  cl::NDRange(64, 64), NULL, &ev);
    ev.wait();

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    cl::size_t<3> region;
    region[0] = 800;
    region[1] = 400;
    region[2] = 1;

    Host::instance()._queue->enqueueReadImage(im, CL_TRUE, origin, region, 0, 0, NULL, NULL, NULL);

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
