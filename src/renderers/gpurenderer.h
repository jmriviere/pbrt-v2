/*
 * gpurenderer.h
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#ifndef GPURENDERER_H_
#define GPURENDERER_H_

#include <vector>

#include "pbrt.h"
#include "renderer.h"
#include "shapes/sphere.h"

/*
 * These structures hold data about rays and spheres to be passed to the GPU.
 * The dummy fields are present to account for memory alignment on the GPU.
 * */

// SamplerRenderer Declarations
class GpuRenderer : public Renderer {
public:
    // SamplerRenderer Public Methods
    GpuRenderer(std::vector<Light*> lights,std::vector<Reference<Shape> > primitives, Sampler *s, Camera *c, bool visIds);
    ~GpuRenderer();
    void Render(const Scene *scene);
    Spectrum Li(const Scene *scene, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena,
        Intersection *isect = NULL, Spectrum *T = NULL) const;
    Spectrum Transmittance(const Scene *scene, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena) const;
private:
    // SamplerRenderer Private Data
    bool visualizeObjectIds;
    Sampler *sampler;
    Camera *camera;

    std::vector<Metadata> meta_primitives;
    std::vector<float> primitives;
};

static std::vector<gpu_Ray> generateGpuRays(Sampler* sampler, const Scene* scene,
									   const Camera* camera, std::vector<std::pair<size_t, Sample*> >& samples);

//GpuRenderer *CreateGpuRenderer(const ParamSet &params, Camera *camera);



#endif /* GPURENDERER_H_ */
