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

int lin1_x[1001], lin1_y[1001];//��һ����
int lin2_x[1001], lin2_y[1001];//�ڶ�����
int lin3_x[1001], lin3_y[1001];//��������

int height = 700;//���ڵĸߺͿ�

//��ÿ�δ��ڴ�С��������ʱ����õĻص�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//�����·��ؼ�(ESC)��رմ���
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

//Bresenham�㷨��ǰ����б��m����-1<=m<=1
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

//��һ���������������xֵ��yֵ��һ����[-1,1]
float normalize(int input) {
	return float(input) / height;
}

//���ֱ���ϵĵ㲢���ص�ĸ���
int getPoints(int x0, int y0, int x1, int y1,int x[],int y[]) {
	int dx, dy;
	int size;//ֱ���ܵ���
	//б�ʲ����ڵ��������ֱ����y��ƽ�л����غ�
	if (x0 == x1) {
		//��֤y1ʼ�մ��ڻ����y0
		if (y0 > y1) {
			int temp = y0;
			y0 = y1;
			y1 = temp;
		}
		size = y1 - y0 + 1;//����ֱ���ܵ���
		//����ֱ����ÿ����ĺ�������
		for (int i = 0; i < size; i++) {
			x[i] = x0;
			y[i] = y0 + i;
		}
	}
	//б�ʴ���
	else {
		float m = float(y1 - y0) / float(x1 - x0);
		//б��m����-1 <= m <= 1
		if (fabs(m) <= 1) {
			//Ĭ��x1����x0
			if (x0 > x1) {
				int temp = x0;
				x0 = x1;
				x1 = temp;
				temp = y0;
				y0 = y1;
				y1 = temp;
			}
			size = x1 - x0 + 1;//����ֱ���ܵ���
			//����ֱ����ÿ��ĺ�����
			for (int i = 0; i < size; i++) {
				x[i] = x0 + i;
			}
			//���б��m����0 <= m <= 1
			if (m >= 0 && m <= 1) {
				y[0] = y0;
				y[size - 1] = y1;
				dx = x1 - x0;
				dy = y1 - y0;
				int p0 = 2 * dy - dx;
				bresenham(y, p0, 0, size, dx, dy);
			}
			//���б��m����-1 <= m < 0,ֻ��Ҫ��m����0 <= m <= 1�ٹ���x��ԳƼ���
			else {
				y[0] = -1 * y0;
				y[size - 1] = -1 * y1;
				dx = x1 - x0;
				dy = -(y1 - y0);
				int p0 = 2 * dy - dx;
				bresenham(y, p0, 0, size, dx, dy);
				//����x��Գ�
				for (int i = 0; i < size; i++) {
					y[i] *= -1;
				}
			}
		}
		//б��m����m > 1 �� m < -1����ʱ��x����y��y����x����
		else {
			//Ĭ��y1����y0
			if (y0 > y1) {
				int temp = y0;
				y0 = y1;
				y1 = temp;
				temp = x0;
				x0 = x1;
				x1 = temp;
			}
			size = y1 - y0 + 1;
			//����ֱ����ÿ���������
			for (int i = 0; i < size; i++) {
				y[i] = y0 + i;
			}
			//���б��m����m > 1
			if (m >= 1) {
				x[0] = x0;
				x[size - 1] = x1;
				dx = x1 - x0;
				dy = y1 - y0;
				int p0 = 2 * dx - dy;
				bresenham(x, p0, 0, size, dy, dx);
			}
			//���б��m����m < -1,ֻ��Ҫ��m����m > 1�ٹ���x��ԳƼ���
			else {
				x[0] = -1 * x0;
				x[size - 1] = -1 * x1;
				dx = -(x1 - x0);
				dy = y1 - y0;
				int p0 = 2 * dx - dy;
				bresenham(x, p0, 0, size, dy, dx);
				//����x��Գ�
				for (int i = 0; i < size; i++) {
					x[i] *= -1;
				}
			}
		}
	}
	return size;
}

//���㺯��
void drawPoints(float fx, float fy, Shader shader) {
	float vertices[] = {
		fx, fy, 0.0f,   0.0f, 0.0f, 0.0f
	};
	unsigned int points_VBO;//���㻺�����
	unsigned int points_VAO;//�����������
	glGenVertexArrays(1, &points_VAO);//����һ��VAO����
	glGenBuffers(1, &points_VBO);//����һ��VBO����
	glBindVertexArray(points_VAO);//��VAO
	//�Ѷ������鸴�Ƶ������й�OpengGLʹ��
	glBindBuffer(GL_ARRAY_BUFFER, points_VBO);//���´����Ļ���VBO�󶨵�GL_ARRAY_BUFFERĿ����
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//��֮ǰ����Ķ�������points_vertices���Ƶ�������ڴ���

    //���Ӷ�������
	//λ�����ԣ�ֵΪ0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//������������
	glEnableVertexAttribArray(0);
	//��ɫ���ԣ�ֵΪ1
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));//������������
	glEnableVertexAttribArray(1);
	shader.use();//������ɫ���������
	glBindVertexArray(points_VAO);//��VAO
	glDrawArrays(GL_POINTS, 0, 1);//����ͼԪ
	//glBindVertexArray(0);
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);
}

//���ߺ���
void drawLines(int size, float fx[], float fy[], Shader shader) {
	for (int i = 0; i < size; i++) {
		drawPoints(fx[i], fy[i], shader);
	}
}

//��Բ����
void drawCircle(int r, float cx, float cy, Shader shader) {
	//��һ��
	float fcx = normalize(cx);
	float fcy = normalize(cy);

	int x = 0, y = r;
	int d = 1 - r;//���(0,R),��һ�е�(1,R - 0.5��,d=1*1+(R-0.5)*(R-0.5)-R*R=1.25-R,dֻ�����������㣬����С�����ֿ�ʡ��
	
	while (y >= x) {
		float fx = normalize(x);
		float fy = normalize(y);

		//���Ƶ�(x,y) (-x,-y) (-x,y) (x,-y)
		drawPoints(fcx + fx, fcy + fy,shader);
		drawPoints(fcx - fx, fcy - fy, shader);
		drawPoints(fcx - fx, fcy + fy, shader);
		drawPoints(fcx + fx, fcy - fy, shader);

		//���Ƶ�(x,y) (-x,-y) (-x,y) (x,-y)
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

//��������κ���
void fillTriangle(int point1[], int point2[], int point3[], float color[], Shader shader) {
	//ֱ�߷���Ϊkx - y + b = 0
	float k1 = (float)(point3[1] - point2[1]) / (float)(point3[0] - point2[0]);
	float k2 = (float)(point1[1] - point3[1]) / (float)(point1[0] - point3[0]);
	float k3 = (float)(point2[1] - point1[1]) / (float)(point2[0] - point1[0]);

	float b1 = (float)point2[1] - k1 * point2[0];
	float b2 = (float)point3[1] - k2 * point3[0];
	float b3 = (float)point1[1] - k3 * point1[0];


	//�ҳ���Χ�����ε���С����
	int xMIN, xMAX, yMIN, yMAX;//��С���ε��ĸ�����
	xMIN = getMin(point1[0], point2[0], point3[0]);
	xMAX = getMax(point1[0], point2[0], point3[0]);
	yMIN = getMin(point1[1], point2[1], point3[1]);
	yMAX = getMax(point1[1], point2[1], point3[1]);

	//���������е�ÿ���㣬����������������������ɫ
	for (int i = xMIN; i <= xMAX; i++) {
		for (int j = yMIN; j <= yMAX; j++) {
			float u = (k1 * i - j + b1) * (k1 * point1[0] - point1[1] + b1);
			float v = (k2 * i - j + b2) * (k2 * point2[0] - point2[1] + b2);
			float w = (k3 * i - j + b3) * (k3 * point3[0] - point3[1] + b3);
			//����������
			if (!(u < 0.0f || v < 0.0f || w < 0.0f)) {
				//��һ��
				float fx = normalize(i);
				float fy = normalize(j);

				//�����ɫ

				float vertices[] = {
					fx, fy, 0.0f,   color[0], color[1], color[2]
				};
				unsigned int points_VBO;//���㻺�����
				unsigned int points_VAO;//�����������
				glGenVertexArrays(1, &points_VAO);//����һ��VAO����
				glGenBuffers(1, &points_VBO);//����һ��VBO����
				glBindVertexArray(points_VAO);//��VAO
				//�Ѷ������鸴�Ƶ������й�OpengGLʹ��
				glBindBuffer(GL_ARRAY_BUFFER, points_VBO);//���´����Ļ���VBO�󶨵�GL_ARRAY_BUFFERĿ����
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//��֮ǰ����Ķ�������points_vertices���Ƶ�������ڴ���

				//���Ӷ�������
				//λ�����ԣ�ֵΪ0
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//������������
				glEnableVertexAttribArray(0);
				//��ɫ���ԣ�ֵΪ1
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));//������������
				glEnableVertexAttribArray(1);
				shader.use();//������ɫ���������
				glBindVertexArray(points_VAO);//��VAO
				glDrawArrays(GL_POINTS, 0, 1);//����ͼԪ
				glBindVertexArray(0);
				glDeleteVertexArrays(1, &points_VAO);
				glDeleteBuffers(1, &points_VBO);
			}
		}
	}
}

int main() {
	glfwInit();//��ʼ��GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//����GLFW���汾��Ϊ3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//����GLFW�ΰ汾��Ϊ3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//ʹ�ú���ģʽ
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);(MACϵͳ���ע��)

	GLFWwindow* window = glfwCreateWindow(height, height, "Bresenham_withGUI", NULL, NULL);//�������ڶ���
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1);

	//�����κ�OpenGL�ĺ���֮ǰ��Ҫ��ʼ��GLAD
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

	bool my_tool_active = true;//���ô����Ƿ���Ч
	bool line = false;//�����Ƿ�ֱ��
	bool circle = false;//�����Ƿ�Բ
	ImVec4 change_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//imGUI��ɫ�����ɵ���ɫ

	glViewport(0, 0, height, height);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	Shader ourShader("shader.vs", "shader.fs");//���춥����ɫ����Ƭ����ɫ��������

	//���������ε�����ֱ��
	int x0 = -100, x1 = 0, x2 = 100, y0 = 0, y1 = 100, y2 = 0;//������ÿ��ֱ�ߵ���������
	int size1, size2, size3;//������ÿ��ֱ���ϵĶ�����Ŀ
	float fx1[1001];//�����ε�һ��ֱ�������е�ĺ���������
	float fy1[1001];//�����ε�һ��ֱ�������е������������
	float fx2[1001];//�����εڶ���ֱ�������е�ĺ���������
	float fy2[1001];//�����ε�һ��ֱ�������е������������
	float fx3[1001];//�����ε�����ֱ�������е�ĺ���������
	float fy3[1001];//�����ε�һ��ֱ�������е������������

	int point1[2], point2[2], point3[2];

	//����Բ�İ뾶��Բ�ĺ����꣬Բ��������
	int r = 100, cx = 0, cy = 0;

	//���GLFW��Ҫ���˳������ѭ��
	while (!glfwWindowShouldClose(window))
	{
		//����
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		//-----------------��Ⱦָ��-----------------
		static float f = 0.0f;

		//����˵���
		ImGui::Begin("bresenham", &my_tool_active, ImGuiWindowFlags_MenuBar);//��ʼGUI������Ⱦ
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
					glfwSetWindowShouldClose(window, true);//�رմ���
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// GUI���ò���
		if (line) {
			ImGui::SliderInt("x1", &x0, -height, height);            // ���õ�һ������ĺ����꣬��ΧΪ��-500��500��
			ImGui::SliderInt("y1", &y0, -height, height);            // ���õ�һ������������꣬��ΧΪ��-500��500��
			ImGui::SliderInt("x2", &x1, -height, height);            // ���õڶ�������ĺ����꣬��ΧΪ��-500��500��
			ImGui::SliderInt("y2", &y1, -height, height);            // ���õڶ�������������꣬��ΧΪ��-500��500��
			ImGui::SliderInt("x3", &x2, -height, height);            // ���õ���������ĺ����꣬��ΧΪ��-500��500��
			ImGui::SliderInt("y3", &y2, -height, height);            // ���õ���������������꣬��ΧΪ��-500��500��
			ImGui::ColorEdit3("change color", (float*)&change_color); // ���������������ɫ
		}
		if (circle) {
			ImGui::SliderInt("cx", &cx, -height, height);            // ����ԲԲ�ĵĺ����꣬��ΧΪ��-500��500��
			ImGui::SliderInt("cy", &cy, -height, height);            // ����ԲԲ�ĵ������꣬��ΧΪ��-500��500��
			ImGui::SliderInt("radium", &r, 0, height);            // ����Բ�İ뾶����ΧΪ��0��500�� 
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
		ImGui::End();//����GUI������Ⱦ

		//��Ⱦ
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//���������Ļ���õ���ɫ
		glClear(GL_COLOR_BUFFER_BIT);//�����Ļ����ɫ����

		
		//��������
		if (line) {
			point1[0] = x0;
			point1[1] = y0;
			point2[0] = x1;
			point2[1] = y1;
			point3[0] = x2;
			point3[1] = y2;
			//��һ����
			size1 = getPoints(x0, y0, x1, y1, lin1_x, lin1_y);//���ֱ���ϵĵ�
			//�����е���й�һ��
			for (int i = 0; i < size1; i++) {
				fx1[i] = normalize(lin1_x[i]);
				fy1[i] = normalize(lin1_y[i]);
			}
			//�ڶ�����
			size2 = getPoints(x0, y0, x2, y2, lin2_x, lin2_y);//���ֱ���ϵĵ�
			//�����е���й�һ��
			for (int i = 0; i < size2; i++) {
				fx2[i] = normalize(lin2_x[i]);
				fy2[i] = normalize(lin2_y[i]);
			}
			//��������
			size3 = getPoints(x1, y1, x2, y2, lin3_x, lin3_y);//���ֱ���ϵĵ�
			//�����е���й�һ��
			for (int i = 0; i < size3; i++) {
				fx3[i] = normalize(lin3_x[i]);
				fy3[i] = normalize(lin3_y[i]);
			}

			//���Ƶ�һ����
			drawLines(size1, fx1, fy1, ourShader);

			//���Ƶڶ�����
			drawLines(size2, fx2, fy2, ourShader);

			//���Ƶ�������
			drawLines(size3, fx3, fy3, ourShader);

			//���������
			float color[3] = { change_color.x,change_color.y,change_color.z };
			fillTriangle(point1, point2, point3, color, ourShader);
		}
		

		//��Բ
		if (circle) {
			drawCircle(r, cx, cy, ourShader);
		}

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		//��鲢�����¼�����������
		glfwSwapBuffers(window);//������ɫ����
		glfwPollEvents();//�����û�д���ʲô�¼�������������롢����ƶ��ȣ������´���״̬�������ö�Ӧ�Ļص�����������ͨ���ص������ֶ����ã���
	}
	//�ͷ���Դ;
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();//��ȷ�ͷ�/ɾ��֮ǰ�ķ����������Դ

	return 0;
}