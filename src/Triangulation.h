#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "imgui.h"

void drawTriangulation(std::vector<Triangle> triangulation)
{
    auto draw = ImGui::GetBackgroundDrawList();

    for (auto& triangle : triangulation)
    {
        draw->AddLine(triangle.p1, triangle.p2, IM_COL32(0, 0, 0, 255));
        draw->AddLine(triangle.p2, triangle.p3, IM_COL32(0, 0, 0, 255));
        draw->AddLine(triangle.p3, triangle.p1, IM_COL32(0, 0, 0, 255));
    }
}

static const float epsilon = 0.0000000001f;

// Shoelace Formula
float polygonArea(const std::vector<ImVec2>& polygon)
{

    int n = polygon.size();

    float area = 0.0f;

    for (int p = n - 1, q = 0; q < n; p = q++)
    {
        area += polygon[p].x * polygon[q].y - polygon[q].x * polygon[p].y;
    }
    return area * 0.5f;
}

// Cross
bool insideTriangle(ImVec2 a, ImVec2 b, ImVec2 c, ImVec2 p)

{
    ImVec2 A, B, C, Ap, Bp, Cp;
    float cCROSSap, bCROSScp, aCROSSbp;

    A.x = c.x - b.x;  A.y = c.y - b.y;
    B.x = a.x - c.x;  B.y = a.y - c.y;
    C.x = b.x - a.x;  C.y = b.y - a.y;
    Ap.x = p.x - a.x;  Ap.y = p.y - a.y;
    Bp.x = p.x - b.x;  Bp.y = p.y - b.y;
    Cp.x = p.x - c.x;  Cp.y = p.y - c.y;

    aCROSSbp = A.x * Bp.y - A.y * Bp.x;
    cCROSSap = C.x * Ap.y - C.y * Ap.x;
    bCROSScp = B.x * Cp.y - B.y * Cp.x;

    return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};

bool snip(const std::vector<ImVec2>& polygon, int u, int v, int w, int n, int* V)
{
    int p;
    ImVec2 a, b, c, p0;

    a.x = polygon[V[u]].x;
    a.y = polygon[V[u]].y;

    b.x = polygon[V[v]].x;
    b.y = polygon[V[v]].y;

    c.x = polygon[V[w]].x;
    c.y = polygon[V[w]].y;

    if (epsilon > (((b.x - a.x) * (c.y - a.y)) - ((b.y - a.y) * (c.x - a.x)))) return false;

    for (p = 0; p < n; p++)
    {
        if ((p == u) || (p == v) || (p == w)) continue;
        p0.x = polygon[V[p]].x;
        p0.y = polygon[V[p]].y;
        if (insideTriangle(a, b, c, p0)) return false;
    }

    return true;
}

// Ear Clipping
bool triangulate(const std::vector<ImVec2>& polygon, std::vector<Triangle>& result)
{
    // allocate and initialize list of Vertices in polygon

    int n = polygon.size();
    if (n < 3) return false;

    int* V = new int[n];

    // we want a counter-clockwise polygon in V
    if (0.0f < polygonArea(polygon))
        for (int v = 0; v < n; v++) V[v] = v;
    else
        for (int v = 0; v < n; v++) V[v] = (n - 1) - v;

    int nv = n;

    int count = 2 * nv;   // error detection

    for (int m = 0, v = nv - 1; nv > 2; )
    {
        // if we loop, it is probably a non-simple polygon
        if (0 >= (count--))
        {
            // Triangulate: ERROR - probable bad polygon!
            return false;
        }

        // three consecutive vertices in current polygon <u, v ,w>
        int u = v; if (nv <= u) u = 0;     // previous
        v = u + 1; if (nv <= v) v = 0;     // new v
        int w = v + 1; if (nv <= w) w = 0; // next

        if (snip(polygon, u, v, w, nv, V))
        {
            int a, b, c, s, t;

            // true names of the vertices
            a = V[u]; b = V[v]; c = V[w];

            // output Triangle
            result.push_back(Triangle(polygon[a], polygon[b], polygon[c]));

            m++;

            // remove v from remaining polygon
            for (s = v, t = v + 1; t < nv; s++, t++) V[s] = V[t]; nv--;

            // resest error detection counter
            count = 2 * nv;
        }
    }

    delete V;

    return true;
}

