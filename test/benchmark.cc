/**
 * \file benchmark.cc
 * \brief Benchmark 2D array access for various array types
 * \date 2025
 * \Acknowledgment - This benchmark is inspired by the benchmark in "rarray" 
 **/
// Copyright (c) 2013-2023  Ramses van Zon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <iostream>
#include <vector>
#include <chrono>
#include <boost/multi_array.hpp>
#include <Eigen/Dense>
#include "carray.h"

#ifndef N_ARRAY
	#define N_ARRAY 4096
#endif

#ifndef N_REPEAT
	#define N_REPEAT 3
#endif

constexpr const int n = static_cast<int>(N_ARRAY);
constexpr const int repeat = static_cast<int>(N_REPEAT);

/**
 * \brief   dispatch a given function and measure the elapsed time
 * \param   f Function to execute
 * \param   arg_in Function argument
 * \param   arg_out Function output 
 * \returns Elapsed time in seconds
 **/
double 
dispatch(std::function<double(int)> f, const int& arg_in, double& arg_out) 
{
	auto start = std::chrono::high_resolution_clock::now();
	arg_out =f(arg_in);
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(end - start).count();
}

double 
test_exact(int repeat) 
{
	double check = repeat*(n-1);
	if (2*(repeat/2) == repeat)
		check += (repeat/2)*(3*repeat/2-2);
	else
		check += (repeat-1)*(3*repeat-1)/4;
	return n*n*check;
}

void pass(float*, float*, int&){}

double 
test_carray(int repeat) 
{
	carray<float, 2, 64> a(n, n), b(n, n), c(n, n);
	double d = 0;
	while(repeat--)
	{
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) 
			{
				a[i][j] = static_cast<float>(i+repeat);
				b[i][j] = static_cast<float>(j+repeat/2);
			}
		pass(&a[0][0], &b[0][0], repeat);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				c[i][j] = a[i][j] + b[i][j];
		pass(&c[0][0], &c[0][0], repeat);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				d += c[i][j];
		pass(&c[0][0], reinterpret_cast<float*>(&d), repeat);
	}
	return d;
}

double 
test_static(int repeat) 
{
	double d = 0.0;
	float a[n][n], b[n][n], c[n][n];
	while (repeat--) 
	{
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++) 
			{
				a[i][j] = static_cast<float>(i+repeat);
				b[i][j] = static_cast<float>(j+repeat/2);
			}
		pass(a[0], b[0], repeat);
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				c[i][j] = a[i][j] + b[i][j];
		pass(c[0], c[0], repeat);
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				d += c[i][j];
		pass(c[0], reinterpret_cast<float*>(&d), repeat);
	}
	return d;
}

double 
test_boost(int repeat) 
{
	boost::multi_array<float, 2> a(boost::extents[n][n]), b(boost::extents[n][n]), c(boost::extents[n][n]);
	double d = 0;
	while(repeat--)
	{
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) 
			{
				a[i][j] = static_cast<float>(i+repeat);
				b[i][j] = static_cast<float>(j+repeat/2);
			}
		pass(&a[0][0], &b[0][0], repeat);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				c[i][j] = a[i][j] + b[i][j];
		pass(&c[0][0], &c[0][0], repeat);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				d += c[i][j];
		pass(&c[0][0], reinterpret_cast<float*>(&d), repeat);
	}
	return d;
}

double 
test_eigen(int repeat) 
{
	Eigen::MatrixXf a(n, n), b(n, n), c(n, n);
	double d = 0;
	while(repeat--)
	{
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) 
			{
				a(j, i) = static_cast<float>(i+repeat);
				b(j, i) = static_cast<float>(j+repeat/2);
			}
		pass(&(a(0, 0)), &(b(0, 0)), repeat);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) 
				c(j, i) = a(j, i) + b(j, i);
		pass(&(c(0, 0)), &(c(0, 0)), repeat);
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				d += c(j, i);	
		pass(&(c(0, 0)), reinterpret_cast<float*>(&d), repeat);
	}
	return d;
}

double
test_std_vector(int repeat) 
{
	std::vector<std::vector<float>> a(n, std::vector<float>(n)), b(n, std::vector<float>(n)), c(n, std::vector<float>(n));
	double d = 0;
	while(repeat--)
	{
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) 
			{
				a[i][j] = static_cast<float>(i+repeat);
				b[i][j] = static_cast<float>(j+repeat/2);
			}
		pass(&a[0][0], &b[0][0], repeat);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				c[i][j] = a[i][j] + b[i][j];
		pass(&c[0][0], &c[0][0], repeat);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				d += c[i][j];
		pass(&c[0][0], reinterpret_cast<float*>(&d), repeat);
	}
	return d;
}

int main(int argc, char* argv[]) 
{
	std::cout << "Benchmarking different 2D array representations:\n";

	struct Benchmark 
	{
		std::string name;
		std::function<double(int)> func;
	} benchmarks[] = 
	{
		{"static", test_static},
		{"carray", test_carray},
		{"std::vector", test_std_vector},
		{"boost::multi_array", test_boost},
		{"Eigen", test_eigen}
	};
	
	for (const auto& bench : benchmarks) 
	{
		double output=0;
		double exact = test_exact(repeat);
		double time = dispatch(bench.func, repeat, output);
		std::cout << bench.name << ": " << time << " seconds\n";
		double eps = 1e-6;
		if (fabs(1-output/exact) >= eps)
		{
			std::cerr << "exact: " << exact << '\n' << "output: " << output << std::endl; 
			std::cerr << bench.name << "does not produce expected result" << std::endl;
		}

	}
	
	return 0;
}