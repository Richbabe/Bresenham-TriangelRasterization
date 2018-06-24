#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <shader_s.h>

using namespace std;

int lin1_x[1001], lin1_y[1001];//第一条线
int lin2_x[1001], lin2_y[1001];//第二条线
int lin3_x[1001], lin3_y[1001];//第三条线

int height = 700;//窗口的高和宽

//在每次窗口大小被调整的时候调用的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//若按下返回键(ESC)则关闭窗口
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

//Bresenham算法，前提是斜率m满足-1<=m<=1
void bresenham(int y[], int p, int i, int size, int dx, int dy) {
	if (i == size - 1) {
		return;
	}
	int pnext;
	if (p <= 0) {
		y[i + 1] = y[i];
		pnext = p + 2 * dy;
	}
	if (p > 0) {
		y[i + 1] = y[i] + 1;
		pnext = p + 2 * dy - 2 * dx;
	}
	bresenham(y, pnext, i + 1, size, dx, dy);
}

//归一化函数，将坐标的x值和y值归一化到[-1,1]
float normalize(int input) {
	return float(input) / height;
}

//获得直线上的点并返回点的个数
int getPoints(int x0, int y0, int x1, int y1,int x[],int y[]) {
	int dx, dy;
	int size;//直线总点数
	//斜率不存在的情况，即直线与y轴平行或者重合
	if (x0 == x1) {
		//保证y1始终大于或等于y0
		if (y0 > y1) {
			int temp = y0;
			y0 = y1;
			y1 = temp;
		}
		size = y1 - y0 + 1;//计算直线总点数
		//计算直线上每个点的横纵坐标
		for (int i = 0; i < size; i++) {
			x[i] = x0;
			y[i] = y0 + i;
		}
	}
	//斜率存在
	else {
		float m = float(y1 - y0) / float(x1 - x0);
		//斜率m满足-1 <= m <= 1
		if (fabs(m) <= 1) {
			//默认x1大于x0
			if (x0 > x1) {
				int temp = x0;
				x0 = x1;
				x1 = temp;
				temp = y0;
				y0 = y1;
				y1 = temp;
			}
			size = x1 - x0 + 1;//计算直线总点数
			//计算直线上每点的横坐标
			for (int i = 0; i < size; i++) {
				x[i] = x0 + i;
			}
			//如果斜率m满足0 <= m <= 1
			if (m >= 0 && m <= 1) {
				y[0] = y0;
				y[size - 1] = y1;
				dx = x1 - x0;
				dy = y1 - y0;
				int p0 = 2 * dy - dx;
				bresenham(y, p0, 0, size, dx, dy);
			}
			//如果斜率m满足-1 <= m < 0,只需要把m看成0 <= m <= 1再关于x轴对称即可
			else {
				y[0] = -1 * y0;
				y[size - 1] = -1 * y1;
				dx = x1 - x0;
				dy = -(y1 - y0);
				int p0 = 2 * dy - dx;
				bresenham(y, p0, 0, size, dx, dy);
				//关于x轴对称
				for (int i = 0; i < size; i++) {
					y[i] *= -1;
				}
			}
		}
		//斜率m满足m > 1 或 m < -1，此时把x看成y，y看成x即可
		else {
			//默认y1大于y0
			if (y0 > y1) {
				int temp = y0;
				y0 = y1;
				y1 = temp;
				temp = x0;
				x0 = x1;
				x1 = temp;
			}
			size = y1 - y0 + 1;
			//计算直线上每点的纵坐标
			for (int i = 0; i < size; i++) {
				y[i] = y0 + i;
			}
			//如果斜率m满足m > 1
			if (m >= 1) {
				x[0] = x0;
				x[size - 1] = x1;
				dx = x1 - x0;
				dy = y1 - y0;
				int p0 = 2 * dx - dy;
				bresenham(x, p0, 0, size, dy, dx);
			}
			//如果斜率m满足m < -1,只需要把m看成m > 1再关于x轴对称即可
			else {
				x[0] = -1 * x0;
				x[size - 1] = -1 * x1;
				dx = -(x1 - x0);
				dy = y1 - y0;
				int p0 = 2 * dx - dy;
				bresenham(x, p0, 0, size, dy, dx);
				//关于x轴对称
				for (int i = 0; i < size; i++) {
					x[i] *= -1;
				}
			}
		}
	}
	return size;
}

//画点函数
void drawPoints(float fx, float fy, Shader shader) {
	float vertices[] = {
		fx, fy, 0.0f,   0.0f, 0.0f, 0.0f
	};
	unsigned int points_VBO;//顶点缓冲对象
	unsigned int points_VAO;//顶点数组对象
	glGenVertexArrays(1, &points_VAO);//生成一个VAO对象
	glGenBuffers(1, &points_VBO);//生成一个VBO对象
	glBindVertexArray(points_VAO);//绑定VAO
	//把顶点数组复制到缓冲中供OpengGL使用
	glBindBuffer(GL_ARRAY_BUFFER, points_VBO);//把新创建的缓冲VBO绑定到GL_ARRAY_BUFFER目标上
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//把之前定义的顶点数据points_vertices复制到缓冲的内存中

    //链接顶点属性
	//位置属性，值为0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//解析顶点数据
	glEnableVertexAttribArray(0);
	//颜色属性，值为1
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));//解析顶点数据
	glEnableVertexAttribArray(1);
	shader.use();//激活着色器程序对象
	glBindVertexArray(points_VAO);//绑定VAO
	glDrawArrays(GL_POINTS, 0, 1);//绘制图元
	//glBindVertexArray(0);
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);
}

//画线函数
void drawLines(int size, float fx[], float fy[], Shader shader) {
	for (int i = 0; i < size; i++) {
		drawPoints(fx[i], fy[i], shader);
	}
}

//画圆函数
void drawCircle(int r, float cx, float cy, Shader shader) {
	//归一化
	float fcx = normalize(cx);
	float fcy = normalize(cy);

	int x = 0, y = r;
	int d = 1 - r;//起点(0,R),下一中点(1,R - 0.5）,d=1*1+(R-0.5)*(R-0.5)-R*R=1.25-R,d只参与整数运算，所以小数部分可省略
	
	while (y >= x) {
		float fx = normalize(x);
		float fy = normalize(y);

		//绘制点(x,y) (-x,-y) (-x,y) (x,-y)
		drawPoints(fcx + fx, fcy + fy,shader);
		drawPoints(fcx - fx, fcy - fy, shader);
		drawPoints(fcx - fx, fcy + fy, shader);
		drawPoints(fcx + fx, fcy - fy, shader);

		//绘制点(x,y) (-x,-y) (-x,y) (x,-y)
		drawPoints(fcx + fy, fcy + fx, shader);
		drawPoints(fcx - fy, fcy - fx, shader);
		drawPoints(fcx - fy, fcy + fx, shader);
		drawPoints(fcx + fy, fcy - fx, shader);

		if (d < 0) {
			d = d + 2 * x + 3;
		}
		else {
			d = d + 2 * (x - y) + 5;
			--y;
		}
		++x;
	}
}

int getMin(int a, int b, int c) {
	return min(a,min(b,c));
}

int getMax(int a, int b, int c) {
	return max(a, max(b, c));
}

//填充三角形函数
void fillTriangle(int point1[], int point2[], int point3[], float color[], Shader shader) {
	//直线方程为kx - y + b = 0
	float k1 = (float)(point3[1] - point2[1]) / (float)(point3[0] - point2[0]);
	float k2 = (float)(point1[1] - point3[1]) / (float)(point1[0] - point3[0]);
	float k3 = (float)(point2[1] - point1[1]) / (float)(point2[0] - point1[0]);

	float b1 = (float)point2[1] - k1 * point2[0];
	float b2 = (float)point3[1] - k2 * point3[0];
	float b3 = (float)point1[1] - k3 * point1[0];


	//找出包围三角形的最小矩形
	int xMIN, xMAX, yMIN, yMAX;//最小矩形的四个顶点
	xMIN = getMin(point1[0], point2[0], point3[0]);
	xMAX = getMax(point1[0], point2[0], point3[0]);
	yMIN = getMin(point1[1], point2[1], point3[1]);
	yMAX = getMax(point1[1], point2[1], point3[1]);

	//遍历矩形中的每个点，如果点在三角形中则填充颜色
	for (int i = xMIN; i <= xMAX; i++) {
		for (int j = yMIN; j <= yMAX; j++) {
			float u = (k1 * i - j + b1) * (k1 * point1[0] - point1[1] + b1);
			float v = (k2 * i - j + b2) * (k2 * point2[0] - point2[1] + b2);
			float w = (k3 * i - j + b3) * (k3 * point3[0] - point3[1] + b3);
			//在三角形中
			if (!(u < 0.0f || v < 0.0f || w < 0.0f)) {
				//归一化
				float fx = normalize(i);
				float fy = normalize(j);

				//填充颜色

				float vertices[] = {
					fx, fy, 0.0f,   color[0], color[1], color[2]
				};
				unsigned int points_VBO;//顶点缓冲对象
				unsigned int points_VAO;//顶点数组对象
				glGenVertexArrays(1, &points_VAO);//生成一个VAO对象
				glGenBuffers(1, &points_VBO);//生成一个VBO对象
				glBindVertexArray(points_VAO);//绑定VAO
				//把顶点数组复制到缓冲中供OpengGL使用
				glBindBuffer(GL_ARRAY_BUFFER, points_VBO);//把新创建的缓冲VBO绑定到GL_ARRAY_BUFFER目标上
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//把之前定义的顶点数据points_vertices复制到缓冲的内存中

				//链接顶点属性
				//位置属性，值为0
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//解析顶点数据
				glEnableVertexAttribArray(0);
				//颜色属性，值为1
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));//解析顶点数据
				glEnableVertexAttribArray(1);
				shader.use();//激活着色器程序对象
				glBindVertexArray(points_VAO);//绑定VAO
				glDrawArrays(GL_POINTS, 0, 1);//绘制图元
				glBindVertexArray(0);
				glDeleteVertexArrays(1, &points_VAO);
				glDeleteBuffers(1, &points_VBO);
			}
		}
	}
}

int main() {
	glfwInit();//初始化GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置GLFW主版本号为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置GLFW次版本号为3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心模式
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);(MAC系统解除注释)

	GLFWwindow* window = glfwCreateWindow(height, height, "Bresenham_withGUI", NULL, NULL);//声明窗口对象
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1);

	//调用任何OpenGL的函数之前需要初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	ImGui_ImplGlfwGL3_Init(window, true);

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	bool my_tool_active = true;//设置窗口是否有效
	bool line = false;//设置是否画直线
	bool circle = false;//设置是否画圆
	ImVec4 change_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//imGUI调色板生成的颜色

	glViewport(0, 0, height, height);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	Shader ourShader("shader.vs", "shader.fs");//构造顶点着色器和片段着色器并链接

	//定义三角形的三条直线
	int x0 = -100, x1 = 0, x2 = 100, y0 = 0, y1 = 100, y2 = 0;//三角形每条直线的两个顶点
	int size1, size2, size3;//三角形每条直线上的顶点数目
	float fx1[1001];//三角形第一条直线上所有点的横坐标数组
	float fy1[1001];//三角形第一条直线上所有点的纵坐标数组
	float fx2[1001];//三角形第二条直线上所有点的横坐标数组
	float fy2[1001];//三角形第一条直线上所有点的纵坐标数组
	float fx3[1001];//三角形第三条直线上所有点的横坐标数组
	float fy3[1001];//三角形第一条直线上所有点的纵坐标数组

	int point1[2], point2[2], point3[2];

	//定义圆的半径，圆心横坐标，圆心纵坐标
	int r = 100, cx = 0, cy = 0;

	//如果GLFW被要求退出则结束循环
	while (!glfwWindowShouldClose(window))
	{
		//输入
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		//-----------------渲染指令-----------------
		static float f = 0.0f;

		//定义菜单栏
		ImGui::Begin("bresenham", &my_tool_active, ImGuiWindowFlags_MenuBar);//开始GUI窗口渲染
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Line", "Ctrl+O")) {
					line = true;
					circle = false;
				}
				if (ImGui::MenuItem("Circle", "Ctrl+S")) {
					line = false;
					circle = true;
				}
				if (ImGui::MenuItem("Close", "Ctrl+W")) {
					my_tool_active = false;
					glfwSetWindowShouldClose(window, true);//关闭窗口
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// GUI设置参数
		if (line) {
			ImGui::SliderInt("x1", &x0, -height, height);            // 设置第一个顶点的横坐标，范围为【-500，500】
			ImGui::SliderInt("y1", &y0, -height, height);            // 设置第一个顶点的纵坐标，范围为【-500，500】
			ImGui::SliderInt("x2", &x1, -height, height);            // 设置第二个顶点的横坐标，范围为【-500，500】
			ImGui::SliderInt("y2", &y1, -height, height);            // 设置第二个顶点的纵坐标，范围为【-500，500】
			ImGui::SliderInt("x3", &x2, -height, height);            // 设置第三个顶点的横坐标，范围为【-500，500】
			ImGui::SliderInt("y3", &y2, -height, height);            // 设置第三个顶点的纵坐标，范围为【-500，500】
			ImGui::ColorEdit3("change color", (float*)&change_color); // 设置三角形填充颜色
		}
		if (circle) {
			ImGui::SliderInt("cx", &cx, -height, height);            // 设置圆圆心的横坐标，范围为【-500，500】
			ImGui::SliderInt("cy", &cy, -height, height);            // 设置圆圆心的纵坐标，范围为【-500，500】
			ImGui::SliderInt("radium", &r, 0, height);            // 设置圆的半径，范围为【0，500】 
		}

		// Display contents in a scrolling region
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Created By Richbabe.");
		ImGui::BeginChild("Scrolling");
		/*
		for (int n = 0; n < 50; n++)
		ImGui::Text("%04d: Some text", n);
		*/
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		if (ImGui::GetIO().Framerate > 180) {
			line = false;
		}
		ImGui::EndChild();
		ImGui::End();//结束GUI窗口渲染

		//渲染
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清空屏幕所用的颜色
		glClear(GL_COLOR_BUFFER_BIT);//清空屏幕的颜色缓冲

		
		//画三角形
		if (line) {
			point1[0] = x0;
			point1[1] = y0;
			point2[0] = x1;
			point2[1] = y1;
			point3[0] = x2;
			point3[1] = y2;
			//第一条线
			size1 = getPoints(x0, y0, x1, y1, lin1_x, lin1_y);//获得直线上的点
			//对所有点进行归一化
			for (int i = 0; i < size1; i++) {
				fx1[i] = normalize(lin1_x[i]);
				fy1[i] = normalize(lin1_y[i]);
			}
			//第二条线
			size2 = getPoints(x0, y0, x2, y2, lin2_x, lin2_y);//获得直线上的点
			//对所有点进行归一化
			for (int i = 0; i < size2; i++) {
				fx2[i] = normalize(lin2_x[i]);
				fy2[i] = normalize(lin2_y[i]);
			}
			//第三条线
			size3 = getPoints(x1, y1, x2, y2, lin3_x, lin3_y);//获得直线上的点
			//对所有点进行归一化
			for (int i = 0; i < size3; i++) {
				fx3[i] = normalize(lin3_x[i]);
				fy3[i] = normalize(lin3_y[i]);
			}

			//绘制第一条线
			drawLines(size1, fx1, fy1, ourShader);

			//绘制第二条线
			drawLines(size2, fx2, fy2, ourShader);

			//绘制第三条线
			drawLines(size3, fx3, fy3, ourShader);

			//填充三角形
			float color[3] = { change_color.x,change_color.y,change_color.z };
			fillTriangle(point1, point2, point3, color, ourShader);
		}
		

		//画圆
		if (circle) {
			drawCircle(r, cx, cy, ourShader);
		}

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		//检查并调用事件、交换缓冲
		glfwSwapBuffers(window);//交换颜色缓冲
		glfwPollEvents();//检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，并调用对应的回调函数（可以通过回调方法手动设置）。
	}
	//释放资源;
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();//正确释放/删除之前的分配的所有资源

	return 0;
}