#pragma once

#include <random>
#include <vector>
#include <numbers>

#include "imgui.h"

class Triangle 
{
public:
	ImVec2 p1, p2, p3;
	Triangle(ImVec2 p1, ImVec2 p2, ImVec2 p3) : p1(p1), p2(p2), p3(p3) {}
};

int is_left(const ImVec2& p0,
	const ImVec2& p1,
	const ImVec2& point)
{
	return ((p1.x - p0.x) * (point.y - p0.y) -
		(point.x - p0.x) * (p1.y - p0.y));
}

bool winding_number(const ImVec2& point,
	const std::vector<ImVec2>& points_list)
{
	int winding_number = 0;

	typedef std::vector<ImVec2>::size_type size_type;

	size_type size = points_list.size();

	for (size_type i = 0; i < size; ++i)
	{
		ImVec2 point1(points_list[i]);
		ImVec2 point2;

		if (i == (size - 1))
		{
			point2 = points_list[0];
		}
		else
		{
			point2 = points_list[i + 1];
		}

		if (point1.y <= point.y)
		{
			if (point2.y > point.y)
			{
				if (is_left(point1, point2, point) > 0)
				{
					++winding_number;
				}
			}
		}
		else
		{
			if (point2.y <= point.y)
			{
				if (is_left(point1, point2, point) < 0)
				{
					--winding_number;
				}
			}
		}
	}

	return (winding_number != 0);
}

bool ray_casting(ImVec2 point, std::vector<ImVec2> polygon)
{
	int count = 0;

	for (int p = polygon.size() - 1, q = 0; q < polygon.size(); p = q++)
	{
		if (point.y < polygon[p].y != point.y < polygon[q].y &&
			(point.x < ((polygon[q].x - polygon[p].x) * (point.y - polygon[p].y) / (polygon[q].y - polygon[p].y) + polygon[p].x)))
		{
			count++;
		}
	}

	return count % 2 == 0 ? false : true;
}

ImVec2 computeCentroid(std::vector<ImVec2> vertices)
{
	ImVec2 centroid;
	double x0 = 0.0;
	double y0 = 0.0;

	int i = 0;
	for (i = 0; i < vertices.size(); ++i)
	{
		x0 = vertices[i].x;
		y0 = vertices[i].y;
		centroid.x += x0;
		centroid.y += y0;
	}

	centroid.x /= vertices.size();
	centroid.y /= vertices.size();

	return centroid;
}

double get_clockwise_angle(const ImVec2& p, const ImVec2 centroid)
{
	double angle = atan2(centroid.y - p.y, centroid.x - p.x) * 180 / std::numbers::pi;
	return angle;
}

bool sortAngles(const ImVec2& a,
	const ImVec2& b, const ImVec2 centroid)
{
	return (get_clockwise_angle(a, centroid) < get_clockwise_angle(b, centroid));
}

std::vector<ImVec2> generatePolygon(int n, float radius, int width, int height, ImVec2* centroid)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-radius, radius);

	std::vector<ImVec2> points;
	
	for (int i = 0; i < n; ++i) points.push_back(ImVec2(width / 2 + dis(gen), height / 2 + dis(gen)));
	
	*centroid = computeCentroid(points);
	std::sort(points.begin(), points.end(), [centroid](ImVec2 a, ImVec2 b) { return sortAngles(a, b, *centroid); });

	return points;
}

void drawPolygon(std::vector<ImVec2> points, ImVec2 centroid)
{
	auto draw = ImGui::GetBackgroundDrawList();

	for (int p = points.size() - 1, q = 0; q < points.size(); p = q++) draw->AddLine(points[p], points[q], IM_COL32(0, 0, 0, 255));

}

void drawDebug(std::vector<ImVec2> polygon, ImVec2 centroid)
{
	auto draw = ImGui::GetBackgroundDrawList();

	for (auto& vertix : polygon) draw->AddLine(ImVec2(centroid.x, centroid.y), vertix, IM_COL32(255, 255, 255, 100));
}

void generatePoints(std::vector<ImVec2>* points, const int n, const int width, const int height)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);

	points->clear();
	for (int i = 0; i < n; ++i) points->push_back(ImVec2(dis(gen) * width, dis(gen) * height));
}