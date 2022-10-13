#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Geometry.h"
#include "Triangulation.h"

std::vector<ImVec2> in_triangulation(std::vector<Triangle> triangulation, std::vector<ImVec2> points)
{
    std::vector<ImVec2> result;

    for (auto& p : points)
    {
        for (auto& triangle : triangulation)
        {
            bool is_in = insideTriangle(triangle.p1, triangle.p2, triangle.p3, p);

            if (is_in) result.push_back(p);
        }
    }
    return result;
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Point in Polygon", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window." << '\n';
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();

    glViewport(0, 0, 800, 800);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBackground;
    window_flags |= ImGuiWindowFlags_NoResize;

    int width, height;
    static int vertices = 10;
    static float area = 200.0f;
    static int quantity = 2000;
    static bool debug = 0;
    static bool triang = 0;
    static bool loop = 0;
    static int item_current = 0;
    const char* items[] = { "Ray Casting Algorithm", "Triangulation Refinement", "Winding Number Algorithm"};


    glfwGetWindowSize(window, &width, &height);
    ImVec2 centroid;
    std::vector<ImVec2> green;
    std::vector<ImVec2> red;
    std::vector<Triangle> triangulation;
    std::vector<ImVec2> polygon = generatePolygon(vertices, area, width, height, &centroid);
    std::vector<ImVec2> points;
    triangulate(polygon, triangulation);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwGetWindowSize(window, &width, &height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto draw = ImGui::GetBackgroundDrawList();
        if(triang) drawTriangulation(triangulation);
        else drawPolygon(polygon, centroid);

        if (debug) drawDebug(polygon, centroid);

        ImGui::Begin("Settings", nullptr, window_flags);

        if (ImGui::Button("Generate Polygon"))
        {
            polygon = generatePolygon(vertices, area, width, height, &centroid);
            triangulation.clear();
            triangulate(polygon, triangulation);
        }

        ImGui::InputInt("Polygon Vertices", &vertices);
        if (vertices <= 0) vertices = 1;

        ImGui::SliderFloat("Polygon Area", &area, 1.0f, 800.0f, "ratio = %.5f");

        if (ImGui::Button("Generate Points")) generatePoints(&points, quantity, width, height);

        if (ImGui::Button("Remove All Points")) points.clear();

        ImGui::InputInt("Number of Points", &quantity);
        if (quantity <= 0) quantity = 1;

        ImGui::Checkbox("Triangulation", &triang);

        ImGui::Checkbox("Loop Points Generation", &loop);

        ImGui::Checkbox("Debug", &debug);

        ImGui::Combo("Algorithm", &item_current, items, IM_ARRAYSIZE(items));

        ImGui::End();


        ImGui::Begin("Coordinates", nullptr, window_flags);

        if (ImGui::IsMousePosValid()) ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
        else ImGui::Text("Mouse pos: <INVALID>");

        ImGui::End();
        
        if (loop) generatePoints(&points, quantity, width, height);

        if (item_current == 0)
        {
            for (auto& p : points)
            {
                bool is_in = ray_casting(p, polygon);

                if (is_in) draw->AddCircleFilled(p, 2, IM_COL32(0, 255, 0, 255), 0.0f);
                else draw->AddCircleFilled(p, 2, IM_COL32(255, 0, 0, 255), 0.0f);
            }
        }
        else if (item_current == 1)
        {
            green = in_triangulation(triangulation, points);
            red = points;
            for (auto& p : green)
            {
                draw->AddCircleFilled(p, 2, IM_COL32(0, 255, 0, 255), 0.0f);

                red.erase(std::remove_if(red.begin(), red.end(),
                    [p](auto& point) { return point.x == p.x && point.y == p.y; }),
                    red.end());
            }

            for (auto& p : red) draw->AddCircleFilled(p, 2, IM_COL32(255, 0, 0, 255), 0.0f);
        }
        else if (item_current == 2)
        {
            for (auto& p : points)
            {
                bool is_in = winding_number(p, polygon);

                if (is_in) draw->AddCircleFilled(p, 2, IM_COL32(0, 255, 0, 255), 0.0f);
                else draw->AddCircleFilled(p, 2, IM_COL32(255, 0, 0, 255), 0.0f);
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
