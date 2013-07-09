/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#include "GPU.h"
#include "ray_caster.h"
#include "fresnel.h"

float3 transform(float3 r, Transformation t) {
	float3 ret;

	ret.s0 = dot(t.m[0], (float4)(r, 1));
	ret.s1 = dot(t.m[1], (float4)(r, 1));
	ret.s2 = dot(t.m[2], (float4)(r, 1));

	return ret;
}

bool ray_sphere_intersection(Hit* hit, Ray ray, Metadata m_sphere, __global float* prims) {

	Sphere sphere;
	sphere.radius = prims[m_sphere.offset];

	Ray trans = ray;
	trans.origin = transform(ray.origin, m_sphere.fromWorld);

	float A = dot(trans.direction, trans.direction);
	float B = 2 * dot(trans.direction, trans.origin);
	float C = dot(trans.origin, trans.origin) - sphere.radius * sphere.radius;

	float delta = B*B - 4*A*C;

	if (delta < 0) {
		return false;
	}

	float sqrt_d = sqrt(delta);

	float q;

	if (B < 0)
	        q = (-B - sqrt_d)/2.0;
	    else
	        q = (-B + sqrt_d)/2.0;

	float t1 = q/A;
	float t2 = C/q;

	hit->t =  (t1 > t2 ? t1 : t2);

	return true;
}

//A revoir
bool intersect(Hit* hit, Ray ray, __global Metadata* meta_prims, __global float* prims, int nb_prims) {
	float temp;
	hit->t = 1e5;

	for (uint i = 0; i < nb_prims; ++i) {
		switch (meta_prims[i].type) {
		case sphere:
			temp = hit->t;
			if (ray_sphere_intersection(hit, ray, meta_prims[i], prims) && hit->t < temp && hit->t >= 1e-2) {
				hit->id = i;
			}
			else {
				hit->t = temp;
			}
			break;
		default:
			break;
		}
	}
	return hit->t < 1e5;
}


Color lookup(image2d_t env, Metadata meta_env, Ray ray) {
	float3 dir = normalize(ray.direction);
	dir.s2 = (dir.s2 > 1.f ? 1.f : (dir.s2 < -1.f ? -1.f : dir.s2));
	float theta = acos(dir.s2);
	float phi = atan2(dir.s1, dir.s0);
	float x = (phi < 0.f ? phi + 2 * M_PI : phi)/(2 * M_PI);
	float y = theta/M_PI;
	Color c = read_imagef(env, sampler, (float2)(x,y));
	return c;
}

Color radiance(image2d_t env, Ray ray, __global Metadata* meta_prims, __global float* prims,
			   int nb_prims, float rand) {

	Color reflectance = (Color)(1, 1, 1, 1);

	Hit hit;

	uint i = 0;

	while(true) {
		if (i++ >= 2 || !intersect(&hit, ray, meta_prims, prims, nb_prims)) {
			return reflectance * lookup(env, meta_prims[hit.id], ray);
		}
		else {
			if (hit.t < 1e-2)
				continue;
			float3 hitpoint = ray.origin + (hit.t + 1e-1) * ray.direction; // hitpoint in world space
			float3 n = normalize(transform(hitpoint, meta_prims[hit.id].fromWorld));
			ray.origin = hitpoint;
			switch (meta_prims[hit.id].mat) {
			case SPEC:
				ray.direction = reflection(ray, n);
				break;
			case DIFF:
			//default:
				float x = cos(2.f * M_PI * rand) * sqrt(1 - rand * rand);
				float y = sin(2.f * M_PI * rand) * sqrt(1 - rand * rand);
				float z = rand;
				ray.direction = normalize((float3)(x, y, z));
				break;
			case REFR:
			default:
				float3 nl = dot(ray.direction, n) < 0 ? n : -n;
				bool into = dot(n, nl) < 0;
				float3 reflectiondir = reflection(ray, nl);

				float nc=1.f, nt=1.5, nnt=into?nc/nt:nt/nc, ddn=dot(ray.direction, nl), cos2t;

				if ((cos2t=1-nnt*nnt*(1-ddn*ddn))<0){
					ray.direction = reflectiondir;
					continue;
				}

				float3 transmissiondir = refraction(ray, nl, nnt);
				float a=nt-nc, b=nt+nc, R0=a*a/(b*b), c = 1-(into?-ddn:dot(n, transmissiondir));
			    float Re = Rpercent(ray, n, nc, nt);//R0 + (1-R0) * c * c * c * c * c;
			    float Tr = 1.f-Re, P = 0.25 + 0.5 * Re, Rp = Re/P, Tp = Tr/(1-P);

			    if (rand < P) {
			    	reflectance *= Rp;
			    	ray.direction = reflectiondir;
			    }
			    else {
			    	reflectance *= Tp;
			    	ray.direction = transmissiondir;
			    }
				break;
			}
		}
	}
}

__kernel void ray_cast(__global float4* Ls, __global Ray* rays, int nb_prims, __global Metadata* meta_prims,
					   __global float* prims, __global Metadata* meta_light, __read_only image2d_t env,
					   __global float* rands) {

	// Index
	int p = get_global_id(0);

	Hit hit;

	Ls[p] = radiance(env, rays[p], meta_prims, prims, nb_prims, rands[p]);
}
