/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#pragma OPENCL EXTENSION cl_amd_printf : enable

typedef struct s_ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct __attribute__ ((packed)) s_sphere {
	float3 center;
	float radius;
} Sphere;

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

kernel void ray_cast(global float* Ls, global Ray* rays, int nb_prim, global Sphere* spheres) {

/*	Sphere sphere;
	sphere.center = (float3)(0, 0, 0);
	sphere.radius = 0.25;*/

	int p = get_global_id(0);

	int z = 0;

/*	if (0 == p) {
		printf("Sphere is as follows: \nRadius = %f \nCenter = {%f, %f, %f}\n", spheres[0].radius,
				spheres[0].center.s0, spheres[0].center.s1, spheres[0].center.s2);
	}*/

/*	printf("Ray n° %d\n Direction = {%f, %f, %f}\n Origin = {%f, %f, %f}\n", p,
			rays[p].direction.s0, rays[p].direction.s1, rays[p].direction.s2,
			rays[p].origin.s0, rays[p].origin.s1, rays[p].origin.s2);*/

	//for (int i = 0; i < nb_prim; ++i) {
		//printf("%f\n", ray_sphere_intersection(rays[p], spheres[0]));
		if (ray_sphere_intersection(rays[p], spheres[0]) != -1) {
			Ls[p] = 1.0;
		}
		else {
			Ls[p] = 0.0;
		}
	//}

}
