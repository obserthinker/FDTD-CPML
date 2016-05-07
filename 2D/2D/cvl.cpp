#include "cvl.h"
//#define debug

/*Class COE*/
COE::COE(int number_layer, src s)
{
	num_layer = number_layer;
	d = number_layer*s.dz;
	sigma_max = (m + 1) / (sqrt(eps_r) * 150 * PI * s.dz);
	fstream myfile;
	myfile.open(filename, ios::out);
	myfile.close();
}

//the distance of every field points of H and E in CPML layer.
float COE::set_distance(src s, float ycoor, float xcoor)
{
	float bench_x, bench_y;
	//for bottom left & right corner
	if (ycoor >= 0.f && ycoor < num_layer)
	{
		if (xcoor >= 0.f && xcoor < num_layer){
			bench_x = num_layer;
			bench_y = num_layer;
			distance = sqrtf(
				powf(fabsf(bench_x - xcoor), 2) + powf(fabsf(bench_y - ycoor), 2)
				);
			return distance*s.dz;
		}
		if (xcoor > (num_layer + s.size_x) && xcoor <= (2 * num_layer + s.size_x)){
			bench_x = num_layer + s.size_x;
			bench_y = num_layer;
			distance = sqrtf(
				powf(fabsf(bench_x - xcoor), 2) + powf(fabsf(bench_y - ycoor), 2)
				);
			return distance*s.dz;
		}
	}
	//for bottom and top center
	if (xcoor >= num_layer && xcoor <= (num_layer + s.size_x))
	{
		//for bottom center area
		if (ycoor >= 0.f && ycoor < num_layer){
			bench_y = num_layer;
			distance = fabsf(bench_y - ycoor);
			return distance*s.dz;
		}
		//for top center area
		if (ycoor > (s.size_y + num_layer) && ycoor <= (s.size_y + 2 * num_layer)){
			bench_y = num_layer + s.size_y;
			distance = fabsf(bench_y - ycoor);
			return distance*s.dz;
		}
	}
	//for left and right center
	if (ycoor >= num_layer && ycoor <= (num_layer + s.size_y)){
		//for left center area
		if (xcoor >= 0.f && xcoor < num_layer){
			bench_x = num_layer;
			distance = fabsf(bench_x - xcoor);
			return distance*s.dz;
		}
		//for right center area
		if (xcoor > (num_layer + s.size_x) && xcoor <= (2 * num_layer + s.size_x)){
			bench_x = num_layer + s.size_x;
			distance = fabsf(bench_x - xcoor);
			return distance*s.dz;
		}
	}
	//for top's left and right corner
	if (ycoor > (num_layer + s.size_y) && ycoor <= (2 * num_layer + s.size_y))
	{
		//left corner
		if (xcoor >= 0.f && xcoor < num_layer){
			bench_x = num_layer;
			bench_y = num_layer + s.size_y;
			distance = sqrtf(
				powf(fabsf(bench_x - xcoor), 2) + powf(fabsf(bench_y - ycoor), 2)
				);
			return distance*s.dz;
		}
		if (xcoor > num_layer + s.size_x && xcoor <= (2 * num_layer + s.size_x)){
			bench_y = s.size_y + num_layer;
			bench_x = s.size_x + num_layer;
			distance = sqrtf(
				powf(fabsf(bench_x - xcoor), 2) + powf(fabsf(bench_y - ycoor), 2)
				);
			return distance*s.dz;
		}
	}
	//for FDTD area
	if ((xcoor >= num_layer && xcoor <= num_layer + s.size_x)
		&& (ycoor >= num_layer&&ycoor <= num_layer + s.size_y)){
		return 0;
	}
}

float COE::set_sigma(src s, float ycoor, float xcoor)
{
	sigma = sigma_max*powf(set_distance(s, ycoor, xcoor) / d, m);
	return sigma;
}

float COE::set_kappa(src s, float ycoor, float xcoor)
{
	//kappa=1;
	kappa = 1 + (kappa_max - 1)*powf(set_distance(s, ycoor, xcoor) / d, m);
	return kappa;
}

float COE::set_alpha(src s, float ycoor, float xcoor)
{
	alpha = set_sigma(s, ycoor, xcoor) / (eps0*set_kappa(s, ycoor, xcoor));
	return alpha;
}

float COE::set_c(src s, float ycoor, float xcoor)
{
	c = (1 / set_kappa(s, ycoor, xcoor))
		*(exp(-set_alpha(s, ycoor, xcoor)*s.dt) - 1);
	return c;
}

float COE::set_coe(src s, float ycoor, float xcoor)
{
	cvl_coe = exp(-set_alpha(s, ycoor, xcoor)*s.dt);
	return cvl_coe;
}

void COE::checkout(src s)
{
	//for coefficient
	cout << "convolution class checkout:" << endl;
	cout << "\tm = " << m << endl;
	cout << "\teps_r = " << eps_r << endl;
	cout << "\teps0 = " << eps0 << endl;
	cout << "\tsigma_max = " << sigma_max << endl;
	cout << "\tkappa_max = " << kappa_max << endl;
	cout << "\td = " << d << endl;
	cout << endl;

	int i, j;
	fstream myfile;
	myfile.open(filename, ios::app);
	for (i = 16; i >= 0; i--){
		for (j = 0; j < 17; j++){
			myfile << set_alpha(s, i, j) << "\t";
			//cout << "(" << i << ", " << j << "): " << set_kappa(s, i, j) << "\t";
		}
		myfile << endl;
		//cout << endl;
	}
	myfile.close();
	//for variables
	//int i;
	//cout << "c_full" << endl;
	//for (i = 0; i < num_layer; i++)
	//{
	//	cout << c_full[i] << endl;
	//}
}

COE::~COE()
{
}


/******************* Class COV ************************/
COV::COV()
{
}

COV::~COV()
{
	delete left;
	left = NULL;
	delete right;
	right = NULL;
	delete top;
	top = NULL;
	delete btm;
	btm = NULL;
}

/**************************** Convolutional Terms *******************************/
HXZ::HXZ(COE c, src s)
{
	full = new area(s.size_x + 2 * c.num_layer + 1, s.size_y + 2 * c.num_layer, "hxz.txt");
	//	left = new area(c.num_layer, s.size_y, "hxzl.txt");
	//	right = new area(c.num_layer, s.size_y, "hxzr.txt");
	//	top = new area(s.size_x + 1, c.num_layer, "hxzt.txt");
	//	btm = new area(s.size_x + 1, c.num_layer, "hxzb.txt");
}

HYZ::HYZ(COE c, src s)
{
	full = new area(s.size_x + 2 * c.num_layer, s.size_y + 2 * c.num_layer + 1, "hyz.txt");
	//	left = new area(c.num_layer, s.size_y + 2 * c.num_layer + 1, "hyzl.txt");
	//	right = new area(c.num_layer, s.size_y + 2 * c.num_layer + 1, "hyzr.txt");
	//	top = new area(s.size_x, c.num_layer, "hyzt.txt");
	//	btm = new area(s.size_x, c.num_layer, "hyzb.txt");
}

EXY::EXY(COE c, src s)
{
	full = new area(s.size_x + 2 * c.num_layer + 1, s.size_y + 2 * c.num_layer + 1, "exy.txt");
	//	left = new area(c.num_layer, s.size_y + 2 * c.num_layer + 1, "exyl.txt");
	//	right = new area(c.num_layer, s.size_y + 2 * c.num_layer + 1, "exyr.txt");
	//	top = new area(s.size_x + 1, c.num_layer, "exyt.txt");
	//	btm = new area(s.size_x + 1, c.num_layer, "exyb.txt");
}

EYX::EYX(COE c, src s)
{
	full = new area(s.size_x + 2 * c.num_layer + 1, s.size_y + 2 * c.num_layer + 1, "eyx.txt");
	//	left = new area(c.num_layer, s.size_y + 2 * c.num_layer + 1, "eyxl.txt");
	//	right = new area(c.num_layer, s.size_y + 2 * c.num_layer + 1, "eyxr.txt");
	//	top = new area(s.size_x + 1, c.num_layer, "eyxt.txt");
	//	btm = new area(s.size_x + 1, c.num_layer, "eyxb.txt");
}