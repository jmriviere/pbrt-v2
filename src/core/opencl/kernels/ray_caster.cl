/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#define M_PI           3.14159265358979323846

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
			       CLK_ADDRESS_CLAMP_TO_EDGE |
			       CLK_FILTER_LINEAR;

#pragma OPENCL EXTENSION cl_amd_printf : enable


typedef struct __attribute__ ((packed)) s_ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct __attribute__ ((packed)) s_sphere {
	float3 center;
	float radius;
} Sphere;

typedef float4 Color;
typedef float3 Normal;



float ray_sphere_intersection(Ray ray, Sphere sphere) {

	float A = dot(ray.direction, ray.direction);
	float B = 2 * dot(ray.direction, ray.origin);
	float C = dot(ray.origin, ray.origin) - sphere.radius * sphere.radius;

	float inv2A = 1/(2 * A);

	float delta = B*B - 4*A*C;

	if (delta < 0) {
		return -1;
	}
	else {
		float sqrt_d = sqrt(delta);
		float t1 = (-B - sqrt_d) * inv2A;
		float t2 = (-B + sqrt_d) * inv2A;

		return (t1 > t2 ? t1 : t2);
	}
}

void reflection(Ray* ref, Ray r, Normal n) {
	ref->direction = 2 * dot(r.direction, n) * n - r.direction;
	ref->origin = n;
}

Color map(image2d_t env, Ray ray, float t) {
	Normal n = normalize(ray.origin + t * ray.direction);
	Ray ref;
	reflection(&ref, ray, n);
	float theta = acos(ref.direction.s1);
	float phi = atan2(ref.direction.s0, ref.direction.s2);
	float x = (phi + M_PI)/(2 * M_PI);
	float y = theta/M_PI;
	Color c = read_imagef(env, sampler, (float2)(x,y));
	return c;
}

__kernel void ray_cast(__read_only image2d_t env, __global float4* Ls, __global Ray* rays, int nb_prim, __global Sphere* spheres) {

	int p = get_global_id(0);
	float t;
	//for (int i = 0; i < nb_prim; ++i) {
		//printf("%f\n", ray_sphere_intersection(rays[p], spheres[0]));
		t = ray_sphere_intersection(rays[p], spheres[0]);
		if (t != -1) {
			Color c = map(env, rays[p], t);
			Ls[p] = c;
		}
		else {
		     float theta = acos(rays[p].direction.s1);
		     float phi = atan2(rays[p].direction.s0, rays[p].direction.s2);
		     float x = (phi + M_PI)/(2 * M_PI);
		     float y = theta/M_PI;
		     Color c = read_imagef(env, sampler, (float2)(x,y));
		     Ls[p] = c;
		}
	//}
}
