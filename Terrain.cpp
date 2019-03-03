#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <vector>
#include <iostream>
#include <time.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif


//Globals


//booleans, mainley used to control what keyboard input does
bool wire = false;
bool tStrip = true;
bool light = true;
bool positiveZR = true;
bool positiveZL = false;
bool moveMode = true;
bool light0 = true;
bool light1 = false;
bool sshading = true;
bool genC = true;
bool both = false;

//max x and z values
float xMax = 300;
float zMax = 300;
//camera positions
float camPos[] = {xMax/2,xMax/2+30,zMax+ zMax};	//where the camera is
float camPosT[3] = {};
float eyes[] = {xMax/2,0,zMax/2}; //where the camera is pointing/ looking

//The vector that holds the heights -- HeightMap
std::vector<std::vector<int>> grid;

//Holds the normals for the heightmap
std::vector<std::vector<std::vector<float>>> normals;

//holds the normal for temp purposes
float cNormal[3] = {0,0,1};

//light posiions
float light_pos[] = {0,xMax,0, 1};
float light_pos2[] = {xMax, xMax,zMax, 1};

//ilght 1 values
float amb[4] = {0.5, 0.5, 0.5, 1};
float dif[4] = {0,1,0,1};
float spc[4] = {0, 0, 1, 1};

//light 2 values
float amb2[4] = {0.5, 0.5, 0.5, 1};
float dif2[4] = {0,0,1,1};
float spc2[4] = {0, 0, 1, 1};

//material values
float m_amb[] = {0.0215,0.1745,0.0215,1.0};
float m_diff[] = {1,1,1,1.0};
float m_spec[] = {0.633,0.727811,0.633,1};
float shiny = 0.6 * 128;





//fills the HeightMap with 0 -- a flat plane
void fillGrid(int h, int w){
	grid.clear();
    for(int i = 0; i < h; i++){
        std::vector<int> temp;
        for (int j = 0; j < w; j++){
            temp.push_back(0);
        }
        grid.push_back(temp);
    }
}

//using a fault-like generation algorithm, generates random heights for the height map
//This generates a more rocky like terrain, with more of a pletau in the middle
//and more mountain like terrain arount the edges and corrners
void faultGen(){
	//the number of iterations
	int iterations =(xMax + zMax) + (xMax/3);
	//allows for random points
	srand(time(NULL));
	int pt[2] = {};
	int pt2[2] = {};
	for (int i = 0; i < iterations; i++){
		//picks what sid eof the fault will rise and fall
		int randomside = (int) rand() % (int)2;
		//picks 2 random points
		pt[0] = (int) rand() % (int)xMax;
		pt[1] = (int) rand() % (int)2;
		//sets z value to edge
		if(pt[1] == 1){
			pt[1] = zMax;
		}
		//picks random point
		pt2[0] = (int) rand() % (int)2;
		pt2[1] = (int) rand() % (int)zMax;
		//sets x to edge
		if(pt2[0] == 1){
			pt2[0] = xMax;
		}
		//assures no vertical lines (no dividing by 0)
		if(pt2[0] == pt[0]){
			pt2[0]+=1;
		}
		if(pt2[1] == pt[1]){
			pt[1]+=1;
		}
		//calculates slope and y intercept
		float m = ((pt2[1] - pt[1])/(pt2[0]-pt[0]));
		int b = pt[1] - (m*pt[0]);

		//checks each points position relative to fault
		for (int j = 0; j < xMax; j++){
			for (int k = 0; k < zMax; k++){
				//calculates the point on the line with same x as current check point
				float newpt[2] = {(float)j, ((m* (j)) +b)};
				if (randomside == 0){
					if(k < newpt[1]){
						grid[j][k] +=1;
					}else{
						grid[j][k] -=1;
					}
				}else{
					if(k >= newpt[1]){
						grid[j][k] +=1;
					}else{
						grid[j][k] -=1;
					}
				}
				

			}
		}


	}
	//shifts terrain up
	if(xMax != 50){
		for (int i = 0; i < xMax; i++){
			for(int j = 0; j < zMax; j++){
				grid[i][j] = grid[i][j] +25;
			}
		

		}
	}
}

//circle gen algorithm
void randomGen(){
	//allows for random points
	srand(time(NULL));
	int iterations = ((xMax*zMax/xMax)/2);
	int pt[2] = {};
	int height;
	int circleSize;
	//picks a point for each iteration, and makes a mountin around it.
	for (int i = 0; i < iterations; i++){
		pt[0] = (int) rand() % (int)xMax;
		pt[1] = (int) rand() % (int)zMax;
		height = (int) rand() % (int) 25;
		circleSize = xMax/2;
		for (int j = 0; j < xMax; j++){
			for(int k = 0; k < zMax; k++){
				float pd = ((sqrt(pow(pt[0] - j,2) + pow(pt[1] - k,2))) * 2)/circleSize;
				if (fabs(pd) <= 1.0){
					grid[j][k] = grid[j][k] + (height/2 + cos(pd*3.14)*(height/2));

				}

			}
			
		}

	}

}

//changes the height of one point and adjusts the points around it using circle algorithm
void manualHeight(int x, int z, int up){
	srand(time(NULL));
	int circleSize = xMax/2;
	int height = (int) rand() % (int) 25;
	for (int j = 0; j < xMax; j++){
			for(int k = 0; k < zMax; k++){
				float pd = ((sqrt(pow(x - j,2) + pow(z - k,2))) * 2)/circleSize;
				if (fabs(pd) <= 1.0){
					if(up == 1){
						grid[j][k] = grid[j][k] + (height/2 + cos(pd*3.14)*(height/2));
					}else{
						grid[j][k] = grid[j][k] - (height/2 + cos(pd*3.14)*(height/2));
					}
					

				}
			}
	}
}





//calculates the normals for each point
void normal(int x, int z){
	//on a 2-d grid with 0,0 in the bottem L, x on vertical, and z on the horzontal
	//calculates for each face a point is a part of, then normalizes them and averges them for the vertex normal

	/*
							TOP
	        +   (xMax,0) __________(xMax,zMax)
			^			|          |
			|		   L|          |R
			|			|          |
			x	   (0,0)|__________|(0,zMax)
							BOT
						z------->+
	*/


	//first 4 are cornner cases
	//0,0
	if(x== 0 && z == 0){
		int u[3] = {x - (x+1), grid[x][z] - grid[x+1][z], z - z};
		int v[3] = {x - (x+1), grid[x][z+1] - grid[x+1][z], (z+1) - z};
		int n[3] = {u[1]*v[2] - u[2]*v[1], u[2]*v[0] - u[0]*v[2],u[0]*v[1] - u[1]*v[0]};
		float lenN = sqrt(pow(n[0],2)+pow(n[1],2)+pow(n[2],2));
		cNormal[0] = n[0]/lenN;
		cNormal[1] = n[1]/lenN;
		cNormal[2] = n[2]/lenN;

	}
	//xMax,zMax
	else if(x == xMax-1 && z == zMax - 1){
		int u[3] = {x - x, grid[x][z] - grid[x][z-1], z - (z-1)};
		int v[3] = {(x-1)-x, grid[x-1][z]-grid[x][z-1], z-(z-1)}; 
		int n[3] = {u[1]*v[2] - u[2]*v[1], u[2]*v[0] - u[0]*v[2],u[0]*v[1] - u[1]*v[0]};
		float lenN = sqrt(pow(n[0],2)+pow(n[1],2)+pow(n[2],2));
		cNormal[0] = n[0]/lenN;
		cNormal[1] = n[1]/lenN;
		cNormal[2] = n[2]/lenN;

	}
	//xMax,0
	else if(x == xMax-1 && z == 0){
		int u[3] = {x - (x-1), grid[x][z] - grid[x-1][z], z - z};
		int v[3] = {x - (x-1), grid[x][z+1] - grid[x-1][z], (z+1)-z};
		int n[3] = {u[1]*v[2] - u[2]*v[1], u[2]*v[0] - u[0]*v[2],u[0]*v[1] - u[1]*v[0]};
		float lenN = sqrt(pow(n[0],2)+pow(n[1],2)+pow(n[2],2));
		cNormal[0] = n[0]/lenN;
		cNormal[1] = n[1]/lenN;
		cNormal[2] = n[2]/lenN;
		


	}
	//0,zMax
	else if( x == 0 && z == zMax-1){
		int u[3] = {x-x, grid[x][z] - grid[x][z-1], z - (z-1)};
		int v[3] = {(x + 1) - x, grid[x+1][z] - grid[x][z-1], z-(z-1)};
		int n[3] = {u[1]*v[2] - u[2]*v[1], u[2]*v[0] - u[0]*v[2],u[0]*v[1] - u[1]*v[0]};
		float lenN = sqrt(pow(n[0],2)+pow(n[1],2)+pow(n[2],2));
		cNormal[0] = n[0]/lenN;
		cNormal[1] = n[1]/lenN;
		cNormal[2] = n[2]/lenN;
		

	}
	//next 4 are edgecases

	//Bot
	else if(x == 0){
		int u1[3] = {x-(x+1), grid[x][z-1] - grid[x+1][z-1], (z-1)-(z-1)};
		int v1[3] = {x-(x+1), grid[x][z], z - (z-1)};
		int n1[3] = {u1[1]*v1[2] - u1[2]*v1[1], u1[2]*v1[0] - u1[0]*v1[2],u1[0]*v1[1] - u1[1]*v1[0]};
		float lenN1 = sqrt(pow(n1[0],2)+pow(n1[1],2)+pow(n1[2],2));

		int u2[3] = {x-(x+1), grid[x][z] - grid[x+1][z-1], z - (z-1)};
		int v2[3] = {(x+1)-(x+1), grid[x+1][z] - grid[x+1][z-1], z - (z-1)};
		int n2[3] = {u2[1]*v2[2] - u2[2]*v2[1], u2[2]*v2[0] - u2[0]*v2[2],u2[0]*v2[1] - u2[1]*v2[0]};
		float lenN2 = sqrt(pow(n2[0],2)+pow(n2[1],2)+pow(n2[2],2));

		int u3[3] = {x-(x+1), grid[x][z] - grid[x+1][z], z - z};
		int v3[3] = {x - (x+1), grid[x][z+1] - grid[x+1][z], (z+1)-z};
		int n3[3] = {u3[1]*v3[2] - u3[2]*v3[1], u3[2]*v3[0] - u3[0]*v3[2],u3[0]*v3[1] - u3[1]*v3[0]};
		float lenN3 = sqrt(pow(n3[0],2)+pow(n3[1],2)+pow(n3[2],2));

		cNormal[0] = ((n1[0]/lenN1) + (n2[0]/lenN2) + (n3[0]/lenN3))/3;
		cNormal[1] = ((n1[1]/lenN1) + (n2[1]/lenN2) + (n3[1]/lenN3))/3;
		cNormal[2] = ((n1[2]/lenN1) + (n2[2]/lenN2) + (n3[2]/lenN3))/3;

	}
	//L
	else if (z == 0){

		
		int u1[3] = {x - (x+1), grid[x][z] - grid[x+1][z], z-z};
		int v1[3] = {x-(x+1), grid[x][z+1] - grid[x+1][z], (z+1) - z};
		int n1[3] = {u1[1]*v1[2] - u1[2]*v1[1], u1[2]*v1[0] - u1[0]*v1[2],u1[0]*v1[1] - u1[1]*v1[0]};
		float lenN1 = sqrt(pow(n1[0],2)+pow(n1[1],2)+pow(n1[2],2));

		int u2[3] = {x-(x-1), grid[x][z+1] - grid[x-1][z+1], (z+1) - (z+1)};
		int v2[3] = {x - (x-1), grid[x][z] - grid[x-1][z+1], z - (z+1)};
		int n2[3] = {u2[1]*v2[2] - u2[2]*v2[1], u2[2]*v2[0] - u2[0]*v2[2],u2[0]*v2[1] - u2[1]*v2[0]};
		float lenN2 = sqrt(pow(n2[0],2)+pow(n2[1],2)+pow(n2[2],2));

		int u3[3] = {x - (x-1), grid[x][z] - grid[x-1][z+1], z - (z+1)};
		int v3[3] = {(x-1) - (x-1), grid[x-1][z] - grid[x-1][z+1], z - (z+1)};
		int n3[3] = {u3[1]*v3[2] - u3[2]*v3[1], u3[2]*v3[0] - u3[0]*v3[2],u3[0]*v3[1] - u3[1]*v3[0]};
		float lenN3 = sqrt(pow(n3[0],2)+pow(n3[1],2)+pow(n3[2],2));

		cNormal[0] = ((n1[0]/lenN1) + (n2[0]/lenN2) + (n3[0]/lenN3))/3;
		cNormal[1] = ((n1[1]/lenN1) + (n2[1]/lenN2) + (n3[1]/lenN3))/3;
		cNormal[2] = ((n1[2]/lenN1) + (n2[2]/lenN2) + (n3[2]/lenN3))/3;


	}
	//Top
	else if(x == xMax-1){
		int u1[3] = {x - x, grid[x][z] - grid[x][z+1], z - (z+1)};
		int v1[3] = {(x-1) - x, grid[x-1][z] - grid[x][z+1], z - (z+1)};
		int n1[3] = {u1[1]*v1[2] - u1[2]*v1[1], u1[2]*v1[0] - u1[0]*v1[2],u1[0]*v1[1] - u1[1]*v1[0]};
		float lenN1 = sqrt(pow(n1[0],2)+pow(n1[1],2)+pow(n1[2],2));

		int u2[3] = {(x - 1) - (x-1), grid[x-1][z] - grid[x-1][z-1], z - (z-1)};
		int v2[3] = {x - (x-1), grid[x][z] - grid[x-1][z-1], z - (z-1)};
		int n2[3] = {u2[1]*v2[2] - u2[2]*v2[1], u2[2]*v2[0] - u2[0]*v2[2],u2[0]*v2[1] - u2[1]*v2[0]};
		float lenN2 = sqrt(pow(n2[0],2)+pow(n2[1],2)+pow(n2[2],2));

		int u3[3] = {x - (x-1), grid[x][z] - grid[x-1][z-1], z - (z-1)};
		int v3[3] = {x - (x-1), grid[x][z - 1] - grid[x-1][z-1], (z-1)-(z-1)};
		int n3[3] = {u3[1]*v3[2] - u3[2]*v3[1], u3[2]*v3[0] - u3[0]*v3[2],u3[0]*v3[1] - u3[1]*v3[0]};
		float lenN3 = sqrt(pow(n3[0],2)+pow(n3[1],2)+pow(n3[2],2));

		cNormal[0] = ((n1[0]/lenN1) + (n2[0]/lenN2) + (n3[0]/lenN3))/3;
		cNormal[1] = ((n1[1]/lenN1) + (n2[1]/lenN2) + (n3[1]/lenN3))/3;
		cNormal[2] = ((n1[2]/lenN1) + (n2[2]/lenN2) + (n3[2]/lenN3))/3;


	}
	//R
	else if(z == zMax-1){
		int u1[3] = {(x-1) - (x-1), grid[x-1][z] - grid[x-1][z-1], z-(z-1)};
		int v1[3] = {x-(x-1), grid[x][z] - grid[x-1][z-1], z - (z-1)};
		int n1[3] = {u1[1]*v1[2] - u1[2]*v1[1], u1[2]*v1[0] - u1[0]*v1[2],u1[0]*v1[1] - u1[1]*v1[0]};
		float lenN1 = sqrt(pow(n1[0],2)+pow(n1[1],2)+pow(n1[2],2));

		int u2[3] = {x-(x-1), grid[x][z] - grid[x-1][z-1], z - (z-1)};
		int v2[3] = {x-(x-1), grid[x][z - 1] - grid[x-1][z-1], (z - 1) - (z-1)};
		int n2[3] = {u2[1]*v2[2] - u2[2]*v2[1], u2[2]*v2[0] - u2[0]*v2[2],u2[0]*v2[1] - u2[1]*v2[0]};
		float lenN2 = sqrt(pow(n2[0],2)+pow(n2[1],2)+pow(n2[2],2));

		int u3[3] = {x-(x+1), grid[x][z-1] - grid[x+1][z], (z-1) - z}; 
		int v3[3] = {x - (x+1), grid[x][z] - grid[x+1][z], z-z};
		int n3[3] = {u3[1]*v3[2] - u3[2]*v3[1], u3[2]*v3[0] - u3[0]*v3[2],u3[0]*v3[1] - u3[1]*v3[0]};
		float lenN3 = sqrt(pow(n3[0],2)+pow(n3[1],2)+pow(n3[2],2));

		cNormal[0] = ((n1[0]/lenN1) + (n2[0]/lenN2) + (n3[0]/lenN3))/3;
		cNormal[1] = ((n1[1]/lenN1) + (n2[1]/lenN2) + (n3[1]/lenN3))/3;
		cNormal[2] = ((n1[2]/lenN1) + (n2[2]/lenN2) + (n3[2]/lenN3))/3;


	}
	//Every point that isnt on an edge or cornner
	//Interior
	else{

		int u1[3] = {x - (x+1), grid[x][z] - grid[x+1][z], z - z};
		int v1[3] = {x - (x+1), grid[x][z+1] - grid[x+1][z] , (z+1) - z};
		int n1[3] = {u1[1]*v1[2] - u1[2]*v1[1], u1[2]*v1[0] - u1[0]*v1[2],u1[0]*v1[1] - u1[1]*v1[0]};
		float lenN1 = sqrt(pow(n1[0],2)+pow(n1[1],2)+pow(n1[2],2));

		int u2[3] = {x - (x+1), grid[x][z] - grid[x+1][z-1], z - (z-1)};
		int v2[3] = {(x+1) - (x+1), grid[x+1][z] - grid[x+1][z-1], z - (z-1)};
		int n2[3] = {u2[1]*v2[2] - u2[2]*v2[1], u2[2]*v2[0] - u2[0]*v2[2],u2[0]*v2[1] - u2[1]*v2[0]};
		float lenN2 = sqrt(pow(n2[0],2)+pow(n2[1],2)+pow(n2[2],2));

		int u3[3] = {x - (x+1), grid[x][z-1]-grid[x+1][z-1], (z-1) - (z-1)};
		int v3[3] = {x - (x+1), grid[x][z] - grid[x+1][z-1], z - (z-1)};
		int n3[3] = {u3[1]*v3[2] - u3[2]*v3[1], u3[2]*v3[0] - u3[0]*v3[2],u3[0]*v3[1] - u3[1]*v3[0]};
		float lenN3 = sqrt(pow(n3[0],2)+pow(n3[1],2)+pow(n3[2],2));

		int u4[3] = {x - x, grid[x][z-1]-grid[x][z], (z-1) - z};
		int v4[3] = {(x - 1) - x, grid[x-1][z] - grid[x][z], z - z};
		int n4[3] = {u4[1]*v4[2] - u4[2]*v4[1], u4[2]*v4[0] - u4[0]*v4[2],u4[0]*v4[1] - u4[1]*v4[0]};
		float lenN4 = sqrt(pow(n4[0],2)+pow(n4[1],2)+pow(n4[2],2));

		int u5[3] = {(x - 1) - x, grid[x-1][z] - grid[x][z], z - z};
		int v5[3] = {(x - 1) - x, grid[x-1][z+1] - grid[x][z], (z+1) - z};
		int n5[3] = {u5[1]*v5[2] - u5[2]*v5[1], u5[2]*v5[0] - u5[0]*v5[2],u5[0]*v5[1] - u5[1]*v5[0]};
		float lenN5 = sqrt(pow(n5[0],2)+pow(n5[1],2)+pow(n5[2],2));

		int u6[3] = {(x - 1) - x, grid[x-1][z+1] - grid[x][z], (z+1) - z};
		int v6[3] = {x - x, grid[x][z+1] - grid[x][z] , (z+1) - z};
		int n6[3] = {u6[1]*v6[2] - u6[2]*v6[1], u6[2]*v6[0] - u6[0]*v6[2],u6[0]*v6[1] - u6[1]*v6[0]};
		float lenN6 = sqrt(pow(n6[0],2)+pow(n6[1],2)+pow(n6[2],2));


		cNormal[0] = ((n1[0]/lenN1) + (n2[0]/lenN2) + (n3[0]/lenN3) + (n4[0]/lenN4) + (n5[0]/lenN5) + (n6[0]/lenN6))/6;
		cNormal[1] = ((n1[1]/lenN1) + (n2[1]/lenN2) + (n3[1]/lenN3) + (n4[1]/lenN4) + (n5[1]/lenN5) + (n6[1]/lenN6))/6;
		cNormal[2] = ((n1[2]/lenN1) + (n2[2]/lenN2) + (n3[2]/lenN3) + (n4[2]/lenN4) + (n5[2]/lenN5) + (n6[2]/lenN6))/6;


	}

}

//calaulates the normals for each point in the heightmap
void placeNormals(){
	normals.clear();
	for(int i =0; i < xMax; i++){
		//temp vector to hold normal vectors
		std::vector<std::vector<float>> temp1;
		for (int j = 0; j < zMax; j++){
			//temp vector to hold normal vector
			std::vector<float> normalV;
			normal(i,j);
			normalV.push_back(cNormal[0]);
			normalV.push_back(cNormal[1]);
			normalV.push_back(cNormal[2]);
			temp1.push_back(normalV);

		}
		normals.push_back(temp1);
	}
}


//calls the selected generation function
void generate(){
	if(genC){
		randomGen();
	}else{
		faultGen();
	}
	placeNormals();
}


//init for window 1
void init(void)
{
	glClearColor(0,0,0,0);
	glColor3f(1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 5000);

	
}

//init for window 2
void init2(void){
	glClearColor(0,0,0,0);
	glMatrixMode(GL_PROJECTION);
	glScalef(1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	gluOrtho2D(-1,300,-1,300);
	glViewport(0,300,300,0);
}


//display for window 1
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//sets camera pos
	gluLookAt(camPos[0], camPos[1], camPos[2], eyes[0], eyes[1], eyes[2], 0,1,0);


	glColor3f(1,1,1);
	//runs if lighting is turned on
	if(light){
		//light 1 settings
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		glLightfv(GL_LIGHT0, GL_SPECULAR, spc);

		//light 2 settings
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glLightfv(GL_LIGHT1, GL_AMBIENT, amb2);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, dif2);
		glLightfv(GL_LIGHT1, GL_SPECULAR, spc2);

		//some material settings. The lights give the main color to the scene
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,m_amb);
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,m_diff);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

	}
	
    
	for (int i = 0; i < xMax-1; i++){
		//sets the draw type
		
		if(tStrip){
			glBegin(GL_TRIANGLE_STRIP);
		}else{
			glBegin(GL_QUAD_STRIP);
		}
		//draws points with normals for lighting and colors for no lighting
		for (int j = 0; j < zMax - 1; j++){
			//4 points, either creating 2 triangles or 1 quad
			glColor3ub(grid[i][j],grid[i][j],grid[i][j]);
			glNormal3f(normals[i][j][0], normals[i][j][1], normals[i][j][2]);
			glVertex3i(i,grid[i][j],j);

			glColor3ub(grid[i+1][j],grid[i+1][j],grid[i+1][j]);
			glNormal3f(normals[i+1][j][0], normals[i+1][j][1], normals[i+1][j][2]);
			glVertex3i(i + 1, grid[i+1][j], j);

			glColor3ub(grid[i][j+1],grid[i][j+1],grid[i][j+1]);
			glNormal3f(normals[i][j+1][0], normals[i][j+1][1], normals[i][j+1][2]);
			glVertex3i(i, grid[i][j+1], j+1);

			glColor3ub(grid[i+1][j+1],grid[i+1][j+1],grid[i+1][j+1]);
			glNormal3f(normals[i+1][j+1][0], normals[i+1][j+1][1], normals[i+1][j+1][2]);
			glVertex3i(i+1, grid[i+1][j+1], j+1);
			

		}
		glEnd();

		//this allows for drawing both the filled and wireframe scene
		if(both){
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
			if(tStrip){
				glBegin(GL_TRIANGLE_STRIP);
			}else{
				glBegin(GL_QUAD_STRIP);
			}	
			for (int j = 0; j < zMax - 1; j++){
				glColor3ub(grid[i][j],grid[i][j],grid[i][j]);
				glNormal3f(normals[i][j][0], normals[i][j][1], normals[i][j][2]);
				glVertex3i(i,grid[i][j]+1,j);

				glColor3ub(grid[i+1][j],grid[i+1][j],grid[i+1][j]);
				glNormal3f(normals[i+1][j][0], normals[i+1][j][1], normals[i+1][j][2]);
				glVertex3i(i + 1, grid[i+1][j]+1, j);

				glColor3ub(grid[i][j+1],grid[i][j+1],grid[i][j+1]);
				glNormal3f(normals[i][j+1][0], normals[i][j+1][1], normals[i][j+1][2]);
				glVertex3i(i, grid[i][j+1]+1, j+1);

				glColor3ub(grid[i+1][j+1],grid[i+1][j+1],grid[i+1][j+1]);
				glNormal3f(normals[i+1][j+1][0], normals[i+1][j+1][1], normals[i+1][j+1][2]);
				glVertex3i(i+1, grid[i+1][j+1]+1, j+1);
				

			}
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		}
		

		


		
	}
	//spheres for visual light position
	//They dont do anything special, just move with the light they are representing
		glPushMatrix();
		glTranslatef(light_pos[0],light_pos[1],light_pos[2]);
		glutSolidSphere(3,10,10);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(light_pos2[0],light_pos2[1], light_pos2[2]);
		glutSolidSphere(3,10,10);
		glPopMatrix();

	//switch buffers for a smooth animation
	glutSwapBuffers();
}


//display for second window
void display2(){
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	
	//using grayscale, sets colors of pixels for second window
	for (int i = 0; i < xMax; i++){
		for (int j = 0; j < zMax; j++){
			
			glBegin(GL_POINTS);
			glColor3ub(grid[i][j],grid[i][j],grid[i][j]);
			glVertex2i(i,j);
			glEnd();
			
		}
	}
	

	glutSwapBuffers();
	

}


//keyboard function for window 1
//instructions for what these do are in the main function/printed when the program runs
void keyboard(unsigned char key, int xIn, int yIn)
{
	int mod = glutGetModifiers();
	switch (key)
	{
		case 27:
			exit(0);
			break;
		case 'o':
			camPos[1] = camPos[1] -1;
			eyes[1] = eyes[1] - 1;
			break;
		case 'p':
			camPos[1] = camPos[1] +1;
			eyes[1] = eyes[1] + 1;
			break;

		case 'w':
			if(wire){
				if(both){
					glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
					wire = false;
					both = false;
				}else{
					both = true;
				}
				
			}else{
				glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
				wire = true;
			}
			break;
		case 't':
			tStrip = true;
			break;
		case 'y':
			tStrip = false;
			break;
		case 'r':
			glutSetWindow(3);
			fillGrid(xMax,zMax);
			generate();
			display2();
			glutSetWindow(1);
			break;
		case 'l':
			if(light){
				glDisable(GL_LIGHT0);
				glDisable(GL_LIGHTING);
				light = false;
			}else{
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				light = true;
			}
			break;
		case 'c':
			if(light0){
				light0 = false;
				light1 = true;

			}
			else{
				light0 = true;
				light1 = false;
			}
			break;
		case 'e':
			if(moveMode){
				//moves camera to light edit position
				moveMode = false;
				camPosT[0] = camPos[0];
				camPosT[1] = camPos[1];
				camPosT[2] = camPos[2];
				camPos[0] = xMax/2;
				camPos[1] = xMax;
				camPos[2] = zMax + zMax;
				eyes[1] = xMax/2;
			}else{
				//moves camera back to pre edit mode pos
				moveMode = true;
				camPos[0] = camPosT[0];
				camPos[1] = camPosT[1];
				camPos[2] = camPosT[2];
				eyes[1] = 0;
			}
			break;

		case 's':
			if(sshading){
				sshading = false;
				glShadeModel(GL_FLAT);
			}else{
				sshading = true;
				glShadeModel(GL_SMOOTH);
			}
			break;
		case 'j':
			if(genC){
				genC = false;
			}else{
				genC = true;
			}
			break;


		



			
	}
}

//arrow keys
void special(int key, int x, int y){
	switch (key){
			case GLUT_KEY_UP:
				//moves view to above terrain
				if(moveMode){

					camPos[1]++;

				}else{
					//moves light0 or light1, along x axis
					if(light0){

						if(light_pos[0] == 0 || light_pos[0] == zMax){
							if((light_pos[2] <= (xMax)) && (light_pos[2] >= 0)){
								light_pos[2]--;
								if(light_pos[2] < 0){
									light_pos[2] = 0;
								}
								if(light_pos[2] > xMax){
									light_pos[2] = xMax;
								}
							}
							
						}

					}else{

						if(light_pos2[0] == 0 || light_pos2[0] == zMax){
							if((light_pos2[2] <= (xMax)) && (light_pos2[2] >= 0)){
								light_pos2[2]--;
								if(light_pos2[2] < 0){
									light_pos2[2] = 0;
								}
								if(light_pos2[2] > xMax){
									light_pos2[2] = xMax;
								}
							}
							
						}

					}
				}
			
			break;

			case GLUT_KEY_DOWN:
				if(moveMode){
					//moves view below terrain

					camPos[1]--;

				}else{
					// moves light0 or light1 along x axis
					if(light0){

						if(light_pos[0] == 0 || light_pos[0] == zMax){
							if((light_pos[2] <= (xMax)) && (light_pos[2] >= 0)){
								light_pos[2]++;
								if(light_pos[2] < 0){
									light_pos[2] = 0;
								}
								if(light_pos[2] > xMax){
									light_pos[2] = xMax;
								}
							}
							
						}

					}else{
						if(light_pos2[0] == 0 || light_pos2[0] == zMax){
							if((light_pos2[2] <= (xMax)) && (light_pos2[2] >= 0)){
								light_pos2[2]++;
								if(light_pos2[2] < 0){
									light_pos2[2] = 0;
								}
								if(light_pos2[2] > xMax){
									light_pos2[2] = xMax;
								}
							}
							
						}

					}
				}
				
				break;
			case GLUT_KEY_LEFT:
				//rotates view around scene
				if(moveMode){

					if(camPos[2] == (zMax + zMax)){
						positiveZL = false;
						positiveZR = true;
					}else if(camPos[2] == (0-zMax)){
						positiveZL = true;
						positiveZR = false;
					}
					if(not positiveZL){
						camPos[2]--;
						camPos[0] = -sqrt(pow(xMax+(xMax/2),2) - pow(camPos[2] - (xMax/2),2))+(xMax/2);
					}else{
						camPos[2]++;
						camPos[0] = sqrt(pow(xMax+(xMax/2),2) - pow(camPos[2] - (xMax/2),2))+(xMax/2);
					}
				}
				else{
					//moves light0 or light1 on z axis
					if(light0){

						if(light_pos[2] == 0 || light_pos[2] == xMax){
							if((light_pos[0] <= (zMax)) && (light_pos[0] >= 0)){
								light_pos[0]--;
								if(light_pos[0] < 0){
									light_pos[0] = 0;
								}
								if(light_pos[0] > zMax){
									light_pos[0] = zMax;
								}
							}
							
						}
					}else{

						if(light_pos2[2] == 0 || light_pos2[2] == xMax ){
							if((light_pos2[0] <= (zMax)) && (light_pos2[0] >= 0)){
								light_pos2[0]--;
								
								if(light_pos2[0] < 0){
									light_pos2[0] = 0;
								}
								if(light_pos2[0] > zMax){
									light_pos2[0] = zMax;
								}
							}
							
						}

					}

				}
				
				break;

			case GLUT_KEY_RIGHT:
				//moves view around terrain
				if(moveMode){
					if(camPos[2] == (zMax + zMax)){
						positiveZR = false;
						positiveZL = true;
					}else if(camPos[2] == (0-zMax)){
						positiveZR = true;
						positiveZL = false;
					}
					if(positiveZR){
						camPos[2]++;
						camPos[0] = -sqrt(pow(xMax+(xMax/2),2) - pow(camPos[2] - (xMax/2),2))+(xMax/2);
					}else{
						camPos[2]--;
						camPos[0] = sqrt(pow(xMax+(xMax/2),2) - pow(camPos[2] - (xMax/2),2))+(xMax/2);
					}
					
					
					
				}else{
					//moves light0 or light1 on z axis
					if(light0){
						if(light_pos[2] == 0 || light_pos[2] == xMax ){
							if((light_pos[0] <= (zMax)) && (light_pos[0] >= 0)){
								light_pos[0]++;
								if(light_pos[0] < 0){
									light_pos[0] = 0;
								}
								if(light_pos[0] > zMax){
									light_pos[0] = zMax;
								}
							}
							
						}
					}else{
						if(light_pos2[2] == 0 || light_pos2[2] == xMax ){
							if((light_pos2[0] <= (zMax)) && (light_pos2[0] >= 0)){
								light_pos2[0]++;
								if(light_pos2[0] < 0){
									light_pos2[0] = 0;
								}
								if(light_pos2[0] > zMax){
									light_pos2[0] = zMax;
								}
							}
							
						}
					}
				}
				break;

	}
	
}

//mouse for windiw 2
void mouse2(int btn, int state, int x, int y){
	if(genC){
		//increases/decreases point height if current generation is circles
		if((x < xMax-1 && y < zMax-1)){
			if (btn == GLUT_LEFT_BUTTON){
					if (state == GLUT_DOWN){
						grid[x][y] += 1;
						manualHeight(x,y,1);
						placeNormals();
						glutSetWindow(1);
						display();
						glutSetWindow(3);
					}

			}

				if (btn == GLUT_RIGHT_BUTTON){
					

					if (state == GLUT_DOWN){
						grid[x][y] -= 1;
						manualHeight(x,y,0);
						glutSetWindow(1);
						display();
						glutSetWindow(3);
					}

				}	

		}

	}

		
	
}



void reshape(int w, int h)
{

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(0, w, 0, h);
	gluPerspective(45, (float)((w+0.0f)/h), 1, 5000);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void FPS(int val){
	glutPostRedisplay();
	glutTimerFunc(17, FPS, 0); // 1sec = 1000, 60fps = 1000/60 = ~17
}

void callBackInit(){
	glutDisplayFunc(display);	//registers "display" as the display callback function
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, FPS, 0);
}

void callBackInit2(){
	glutDisplayFunc(display2);	//registers "display2" as the display callback function
	glutMouseFunc(mouse2);
	glutTimerFunc(0, FPS, 0);
}

//menu for size options
void menuProc(int value){
	if(moveMode){
		if (value == 1){
			xMax = 50;
			zMax = 50;
		}
			
		if (value == 2){
			xMax = 100;
			zMax = 100;
		}
			
		if (value == 3){
			xMax = 150;
			zMax = 150;
		}
			
		if (value == 4){
			xMax = 200;
			zMax = 200;
		}
			
		if (value == 5){
			xMax = 250;
			zMax = 250;
		}
			
		if (value == 6){
			xMax = 300;
			zMax = 300;
		}
		//renderes new grid based on selected size
		fillGrid(xMax,zMax);
		generate();

		camPos[0] = xMax/2;
		camPos[1] = xMax/2+30;
		camPos[2] = zMax + zMax;
		eyes[0] = xMax/2;
		eyes[1] = 0;
		eyes[2] = zMax/2;

		light_pos[0] = 0;
		light_pos[1] = xMax;
		light_pos[2] = 0;
		light_pos2[0] = xMax;
		light_pos2[1] = xMax;
		light_pos2[2] = zMax;
		glutSetWindow(1);
		display();
		glutSetWindow(3);
		display2();
		glutSetWindow(1);

	}
	

}

//creates menu
void createMenu(){
	int main_id = glutCreateMenu(menuProc);
	glutAddMenuEntry("50x50",1);
	glutAddMenuEntry("100x100",2);
	glutAddMenuEntry("150x150",3);
	glutAddMenuEntry("200x200",4);
	glutAddMenuEntry("250x250",5);
	glutAddMenuEntry("300x300",6);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


/* main function - program entry point */
int main(int argc, char** argv)
{


	printf("Random Terrain Generator\n");
	printf("The program will have the Overview Window selected by default\n");
	printf("-------When in the Terrain Window-------\n");
	printf("Pressing the ESC key will close the program\n");
	printf("Right clicking will open a menu, with different size options for the grid\n");
	printf("Pressing on one of these options will re generate the terrain at that size\n");
	printf("Use the left and right arrow keys to rotate arount the terrain\n");
	printf("Use the up and down keys to look above or below the terrain\n");
	printf("Use the p and o keys to raise and lower the camera respectively\n");
	printf("Press the r key to generate a new terrain\n");
	printf("Use the j key to switch the terrain generation algorithm-- the r key will have to be used to generate the new terrain\n");
	printf("Use the w key to switch between filled, wireframe and both filled and wireframe terrain\n");
	printf("Use the t key to generate using tirangle strips\n");
	printf("Use the y key to generate using quad strips\n");
	printf("Press the l key to turn lighting on and of\n");
	printf("Pressing the s key will change the shading mode between flat and smooth\n");
	printf("Press the e key to enter light edit mode. This will allow you to move the lights around the grid with the arrow keys\n");
	printf("Pressing the c key will allow you to switch whick light you control in edit mode\n");
	printf("-------While in the Overview Window------\n");
	printf("This window shows an overview of the current generation\n");
	printf("The window is 300x300 (max grid size)\n");
	printf("Left clicking on a point that falls inside the current grid will raise that point on the terrain\n");
	printf("Right clicking on a point that falls inside the current grid will lower that point on the terrain\n");
	printf("These features are only available using the circles/default algorithm for terrain gen\n");
	





	fillGrid(xMax, zMax);
	generate();
	placeNormals();
	glutInit(&argc, argv);		//starts up GLUT
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(800,600);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("Terrain");

	callBackInit();	

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	
	
    glShadeModel(GL_SMOOTH);


	glFrontFace(GL_CCW);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	init();
	createMenu();



	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(300,300);
	glutInitWindowPosition(900, 50);
	glutCreateWindow("Overview");
	callBackInit2();


	init2();


	glutMainLoop();				//starts the event glutMainLoop
	return(0);					//return may not be necessary on all compilers
}