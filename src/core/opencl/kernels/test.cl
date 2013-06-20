
__kernel void test(__global int* i) {

	i[get_global_id(0)] = 150;
	
}