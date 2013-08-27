/*
 * gpurenderer.h
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#ifndef OCLRENDERER_H_
#define OCLRENDERER_H_

#include <vector>

#include "pbrt.h"
#include "renderer.h"
#include "shapes/sphere.h"

// SamplerRenderer Declarations
class OCLRenderer : public Renderer {
public:
    // SamplerRenderer Public Methods
    OCLRenderer(std::vector<Light*> lights,std::vector<Reference<Shape> > primitives,
    			Sampler *s, Camera *c, bool visIds);
    ~OCLRenderer();
    void Render(const Scene *scene);
    Spectrum Li(const Scene *scene, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena,
        Intersection *isect = NULL, Spectrum *T = NULL) const;
    Spectrum Transmittance(const Scene *scene, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena) const;
private:
    Sampler *sampler;
    bool visualizeObjectIds;
    Camera *camera;

    std::vector<Metadata> meta_primitives;
    std::vector<float> primitives;
};



#endif /* GPURENDERER_H_ */
