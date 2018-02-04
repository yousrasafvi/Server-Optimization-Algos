//Author: Yousra Safvi

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#define infinity 9999
using namespace std;

struct point
{
    int x;
    int y;
};

struct edge
{
    struct point v1;
    struct point v2;
};

struct triangle
{
    struct point a;
    struct point b;
    struct point c;
};

// Declaring function that will be required to display calculate and find shortest path t*/
void displayTriangles(int * ver,int tri);
int isPointInTriangle(struct point p , struct triangle t);
int orientation(struct point p, struct point q, struct point r);
int isIntersect(struct point p1,struct point p2,struct point p3, struct point p4);
int isIntersectEdge(struct point v1,struct point v2,struct triangle tri);
void joinPathFromPoints(struct triangle *t,struct point p, struct point t1, int num_tri);
void applyDijkstra(struct triangle *t, struct edge *keep_edges, struct point start, struct point target,int number_of_triangles, int number_of_paths);
int calculateLength(struct edge paths);
int joinVertices( struct triangle *t, int tri, struct edge *keep_edges);

// Variables for opening screen on window.
Display *display;  // this variable will contain the pointer to the Display structure //
Screen *screen_ptr;
char *display_name=NULL;
// these variables will store the size of the screen, in pixels.
unsigned int screen_num;
unsigned int screen_width,screen_height;

Window win;         // this variable will be used to store the ID of the root window of screen
int border_width;
unsigned int win_width, win_height;     // these variables will store the window's height and width.
int win_x, win_y;                       // these variables will store the window's location.

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;
// these variables will be used to store the IDs of the black and white colors of the screen
unsigned long white_pixel;
unsigned long black_pixel;

// variables for the graphics
GC gc, gc_blue, gc_red;
unsigned long valuemask=0;
XGCValues gc_values,gc_red_values,gc_blue_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

int main(int argc, char* argv[])
{
    // open the connection to the X server".
    display = XOpenDisplay(display_name);
    if (display == NULL)
    {
        printf("Cannot connect to X server \n");
        exit (-1);
    }

    screen_num = DefaultScreen(display);    // check the number of the default screen for our X server.
    screen_ptr = DefaultScreenOfDisplay( display );
    color_map  = XDefaultColormap( display, screen_num );
    screen_width = DisplayWidth(display, screen_num);       //find the width of the default screen of our X server, in pixels.
    screen_height = DisplayHeight(display, screen_num);     //find the height of the default screen of our X server, in pixels.

    win = RootWindow(display, screen_num);      // find the ID of the root window of the screen. //

    white_pixel = WhitePixel(display, screen_num);      // find the value of a white pixel on this screen.
    black_pixel = BlackPixel(display, screen_num);      // find the value of a black pixel on this screen.

// create window
// calculate the window's width and height.
    win_width = DisplayWidth(display, screen_num) / 1.7;
    win_height = DisplayHeight(display, screen_num) / 1.2;
    border_width = 10;
// position of the window is top-left corner - 10,10.
    win_x = win_y = 10;
    win = XCreateSimpleWindow(display,
                          RootWindow(display, screen_num),
                          win_x, win_y,
                          win_width, win_height,
                          border_width, BlackPixel(display, screen_num),
                          WhitePixel(display, screen_num));
    // Putting window on screen.
    size_hints = XAllocSizeHints();
    wm_hints = XAllocWMHints();
    class_hints = XAllocClassHint();
    if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
        { printf("Error allocating memory for hints. \n"); exit(-1);}

    size_hints -> flags = PPosition | PSize | PMinSize  ;
    size_hints -> min_width = 60;
    size_hints -> min_height = 60;

    XStringListToTextProperty( &win_name_string,1,&win_name);
    XStringListToTextProperty( &icon_name_string,1,&icon_name);

    wm_hints -> flags = StateHint | InputHint ;
    wm_hints -> initial_state = NormalState;
    wm_hints -> input = False;

    class_hints -> res_name = "x_use_example";
    class_hints -> res_class = "examples";

    XSetWMProperties( display, win, &win_name, &icon_name, argv, argc,
                    size_hints, wm_hints, class_hints );
    XSelectInput( display, win,
            ExposureMask | StructureNotifyMask | ButtonPressMask );

    XMapWindow(display, win);
    XFlush(display);

// create graphics content to draw
    gc = XCreateGC( display, win, valuemask, &gc_values);
    XSetForeground(display, gc, BlackPixel(display, screen_num));
    //XSetBackground(display, gc, BlackPixel(display, screen_num));
    XSetFillStyle(display, gc, FillSolid);
    XSetLineAttributes(display, gc, 2, LineSolid, CapRound, JoinRound);

    gc_blue = XCreateGC( display, win, valuemask, &gc_blue_values);
    XSetLineAttributes(display, gc_blue, 3, LineSolid,CapRound, JoinRound);
    if( XAllocNamedColor( display, color_map, "blue",
			&tmp_color1, &tmp_color2 ) == 0 )
        {printf("failed to get color blue\n"); exit(-1);}
    else
        {XSetForeground( display, gc_blue, tmp_color1.pixel );}

    gc_red = XCreateGC( display, win, valuemask, &gc_red_values);
    XSetLineAttributes(display, gc_red, 2, LineSolid,CapRound, JoinRound);
    if( XAllocNamedColor( display, color_map, "red",
			&tmp_color1, &tmp_color2 ) == 0 )
        {printf("failed to get color red\n"); exit(-1);}
    else
        {XSetForeground( display, gc_red, tmp_color1.pixel );}

    // The next part of the code deals with displaying the triangle obstacles on the screen
    // joining together vertices of the other triangles, that is, finding valid path,calculating length of each path
    // and then applying dijkstra to find shortest path.

    int num_ver=0,num_tri=0; // number of triangle obstacles and number of vertices
    int j = 0,i = 0;              // for loops
    int *store_ver;             //store_ver stores coordiantes of triangle
    store_ver=new int[3000];
    struct triangle t[1000];
    struct point start;
    struct point target;
    static int lclick = -1;
    static int rclick = 0;
    int count_click = 0;
   // opening the file and getting vertices of triangles
    char buf[1024];   // no line is longer than 1024 characters
    char *str;
    FILE *fp;
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        printf("No input file\n");

    }
    else
    {
        while (fgets(buf, 1024,fp)!=NULL)
        {
            str = strtok(buf, "ABCDEFGHIJKLMNOPQRSTUVWXYZ,() \n");
            while (str != NULL)
            {
                store_ver[num_ver]=atoi(str); //array of vertices
                str = strtok(NULL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ,\n ()");
                num_ver++;
            }
            num_tri++; // counts number of triangle
        }
        for(j=0;j<num_tri;j++)
        {
            t[j].a.x = store_ver[i],t[j].a.y = store_ver[i+1];
            t[j].b.x = store_ver[i+2],t[j].b.y = store_ver[i+3];
            t[j].c.x = store_ver[i+4],t[j].c.y = store_ver[i+5];
            i = i + 6;
        }
        fclose(fp);
    }
//end of file
    while(1)
    { XNextEvent( display, &report );
      switch( report.type )
      {
        case Expose:
        // draw the triangles, this happens
        // Each time some part of the window gets exposed (becomes visible)
        {
            displayTriangles(store_ver , num_tri);
        }
        break;
        case ConfigureNotify:
         { // This event happens the size of the window is change.
          win_width = report.xconfigure.width;
          win_height = report.xconfigure.height;
          break;
          }
        case ButtonPress:
          // This event happens when mouse button is click.
        {
            int x1=0,y1=0;
            int x[3],y[3];
            bool result;
            x1 = report.xbutton.x;
            y1 = report.xbutton.y;
            if(lclick== -1) // To draw triangles
            {
                if (report.xbutton.button == Button1)
                {
                    x[(count_click%3)] = x1;
                    y[(count_click%3)] = y1;
                    count_click++;
                    XFillArc( display, win, gc,
                    x1 , y1 ,
                    4, 4, 0, 360*64);
                    if((count_click%3) == 0)
                    {
                        XDrawLine(display, win, gc, x[0], y[0], x[1], y[1]);
                        XDrawLine(display, win, gc, x[1], y[1], x[2], y[2]);
                        XDrawLine(display, win, gc, x[2], y[2], x[0], y[0]);
                        t[num_tri].a.x = x[0], t[num_tri].a.y = y[0];
                        t[num_tri].b.x = x[1], t[num_tri].b.y = y[1];
                        t[num_tri].c.x = x[2], t[num_tri].c.y = y[2];
                        num_tri++;
                    }
                }
                else
                {
                   if(rclick == 0)
                   { lclick = 0, rclick = 1; }
                   else
                   { exit(-1); }
                }
            }
            else if(lclick == 0)
            {
                if (report.xbutton.button == Button1)
                {
                    start.x = x1;
                    start.y = y1;
                    for(i=0;i<num_tri;i++)
                    {
                        result = isPointInTriangle(start,t[i]); // checks if start point is not inside triangles
                        if(result)
                        {
                         printf("Invalid start point\n Re-run from the program");
                         exit(-1);
                        }
                    }
                    XFillArc( display, win, gc_red,
                    start.x-win_height/90 , start.y-win_height/90,
                    win_height/70, win_height/70, 0, 360*64);
                    lclick = 1;
                }
                else
                {
                    if(rclick == 1)
                    { printf("\nYou have clicked right mouse button\n Exiting from the program");
                    exit(-1);}
                }
            }
            else
            {
                if (report.xbutton.button == Button1 )
                {
                    target.x = x1;
                    target.y = y1;
                    for(i = 0; i < num_tri ; i++)
                    {

                        result = isPointInTriangle(target,t[i]); //checks if destination point is not inside triangles
                        if(result)
                        {
                         printf("\nInvalid target point\n Re-run the program");
                          exit(-1);
                        }
                    }
                    XFillArc( display, win, gc_red,
                    target.x-win_height/90 , target.y-win_height/90,
                    win_height/70, win_height/70, 0, 360*64);

                    joinPathFromPoints(t,start, target, num_tri); //calls function to show path.

                    lclick = 0;
                }
                else
                {
                    if(rclick == 1)
                    { printf("\nYou have clicked right mouse button\n Exiting from the program");
                    exit(-1);}
                }
            }

        }
        break;
       }
    }

    return 0;
}
// This function to displays triangle from the file.
//This function reads vertices of triangle from the file triangle_ver and then displays the triangle on the screen.
void displayTriangles(int *ver, int tri)
{
    int i = 0, j=0;
    // displays trianges and also assigns values to the triangle coordinates . This array will be required for further calculation.
    for(j = 0 ; j < tri ; j++)
    {
        {   XPoint points[] = {{ver[i],ver[i+1]},
                                {ver[i+2],ver[i+3]},
                                {ver[i+4],ver[i+5]},
                                {ver[i],ver[i+1]} };
            int npoints = sizeof(points)/sizeof(XPoint);
            XDrawLines(display, win, gc, points, npoints, CoordModeOrigin);
        };
        i = i + 6;
     }
}
// Function checks that a given point or vertex p is in triangle or not.
int isPointInTriangle(struct point p , struct triangle t)
{
//Calculate vector product of point p with respect to vertices of triangles.
    int z_abp = ((p.x - t.a.x) * (t.b.y - t.a.y)) - ((p.y - t.a.y) * (t.b.x - t.a.x));
    int z_bcp = ((p.x - t.b.x) * (t.c.y - t.b.y)) - ((p.y - t.b.y) * (t.c.x - t.b.x));
    int z_cap = ((p.x - t.c.x) * (t.a.y - t.c.y)) - ((p.y - t.c.y) * (t.a.x - t.c.x));
    if ((z_abp > 0) && (z_bcp > 0) && (z_cap > 0)) return 1;                  //If all vector products are negative or all vector products are positive point lies inside triangle.
    else if ((z_abp < 0) && (z_bcp < 0) && (z_cap < 0)) return 1;
    else return 0;
}
// This function returns the orientation a point r with respect to line p,q.
// The function return 0 for clockwise and 1 for anti-clockwise.
int orientation(struct point p, struct point q, struct point r)
{

    int val = (q.y - p.y) * (r.x - q.x) -       //vector product to see orientation
              (q.x - p.x) * (r.y - q.y);

    return (val > 0)? 0: 1;
}
//This function checks if the two line form by vertices p1,p2,p3,p4 intersect each other or not.
int isIntersect(struct point p1,struct point p2,struct point p3, struct point p4)
{

    int o1 = orientation(p1, p2, p3);       // the orientation of the coordinates of first line with respect to second line
    int o2 = orientation(p1, p2, p4);
    int o3 = orientation(p3, p4, p1);       // the orientation of the coordinates of second line with respect to frst line
    int o4 = orientation(p3, p4, p2);
    if (o1 != o2 && o3 != o4)               // the two lines will intersect if and only if their orientation are different
    {return 1;}
    else
    {return 0;}
}

// This function checks if the line between vertices v1 and v2 intersect any of the edge of triangles.
int isIntersectEdge(struct point v1,struct point v2,struct triangle tri)
{
    int r1 = 0,r2 = 0,r3 = 0;
    r1 = isIntersect(v1,v2,tri.a,tri.b);
    r2 = isIntersect(v1,v2,tri.b,tri.c);
    r3 = isIntersect(v1,v2,tri.c,tri.a);
    if((r1 == r2) && (r2 == r3))
        return 1;
    else return 0;
}

/* This function works in three part.
  First part, calls the function joinVertices to form path between all the triangle vertices
  and keep those path which donot intersect any triangle.
  Second part checks for a direct path between source and target point that does not intersect any triangles edges.
  It also find paths from start and target point to all vertices of triangles and keep valid paths.
  Third part calls applydijkstra function that shows the shortest path between start and target point.*/
void joinPathFromPoints(struct triangle *t, struct point p,
                        struct point t1, int num_tri)
{
   int res1 = 0, r1 = 0 , r = 0, temp = 0 ;
   int i = 0, j = 0, k = 0, n =0, f =0, num_of_path = 0;
   n = num_tri;
   int notintersect = 0;
   struct point p1;
   struct edge keep_edges[10000]; // store all the valid path between vertices of triangles,from start point to destination point
                    //and also from start point and destination point to vertices of triangles.

   // Call function joinVertices to find and form all valid paths.
   num_of_path = joinVertices(t, num_tri, keep_edges);
   f=num_of_path;
   //checks for direct path between start and target point.
   for(i = 0; i < n; i++)
   {
        int result = isIntersectEdge(p,t1,t[i]);
        if(!result)
        {notintersect = 0;
        break;}
        else
        {notintersect = 1;}
   }
   if(notintersect == 1)
   {
        keep_edges[f].v1.x = p.x , keep_edges[f].v1.y = p.y;
        keep_edges[f].v2.x = t1.x , keep_edges[f].v2.y = t1.y;
        f++;
   }

// Find and store valid paths(those paths which do not intersect the edges of triangles) from start and target point to vertices of triangles.
   for(k = 0; k < 2; k++)
   {
        p1 = (k == 0) ? p : t1;
        for(i = 0; i < n; i++)
        {

        res1 = isIntersect(p1,t[i].a,t[i].b,t[i].c);  /* check if the line from a point to the vertex 'a' of  triangle[i] does not intersect other edge of the triangle[i] */
        if(!res1)
// check if the same line does not intersect the edges of other triangle obstacles.
        {
            for(j = 0; j < n; j++)
            {
            if(i != j)
            {
                r = isPointInTriangle(t[i].a,t[j]);
                r1 = isIntersectEdge(p1,t[i].a,t[j]);
                if((!r1) || ( r ))
                {
                    temp = 0;
                    break;
                }
                else
                {
                    temp = 1;
                }
            }
            }
            if(temp == 1)
            {
                //XDrawLine(display,win,gc_blue,p1.x,p1.y,t[i].a.x,t[i].a.y);
                keep_edges[f].v1.x = p1.x , keep_edges[f].v1.y = p1.y;
                keep_edges[f].v2.x = t[i].a.x , keep_edges[f].v2.y = t[i].a.y;
                f++;
            }
        }
        res1 = isIntersect(p1,t[i].b,t[i].c,t[i].a);  //check if the line from a point to the vertex 'b' of  triangle[i] does not intersect other edge of the triangle[i] //
        if(!res1)
// check if the same line does not intersect the edges of other triangle obstacles.
        {
            for(j = 0; j < n; j++)
            {
            if(i != j)
            {
                r = isPointInTriangle(t[i].b,t[j]);
                r1 = isIntersectEdge(p1,t[i].b,t[j]);
                if (!r1 || r)
                {
                    temp = 0;
                    break;
                }
                else
                {
                    temp = 1;
                }
            }
           }
           if(temp == 1)
           {
                //XDrawLine(display,win,gc_blue,p1.x,p1.y,t[i].b.x,t[i].b.y);
                keep_edges[f].v1.x = p1.x , keep_edges[f].v1.y = p1.y;
                keep_edges[f].v2.x = t[i].b.x , keep_edges[f].v2.y = t[i].b.y;
                f++;
           }
       }
       res1 = isIntersect(p1,t[i].c,t[i].a,t[i].b);
       if(!res1)
// check if the line does not intersect the edges of other triangle obstacles.
       {
            for( j = 0; j < n; j++)
            {
            if(i != j)
            {
                r = isPointInTriangle(t[i].c,t[j]);
                r1 = isIntersectEdge(p1,t[i].c,t[j]);
                if (!r1 || r)
                {
                  temp = 0;
                  break;
                }
                else
                {
                    temp = 1;
                }
             }
            }
            if (temp == 1)
            {
                //XDrawLine(display,win,gc_blue,p1.x,p1.y,t[i].c.x,t[i].c.y);
                keep_edges[f].v1.x = p1.x , keep_edges[f].v1.y = p1.y;
                keep_edges[f].v2.x = t[i].c.x , keep_edges[f].v2.y = t[i].c.y;
                f++;
            }
        }
    }
  }
    applyDijkstra(t,keep_edges,p,t1,num_tri,f); //apply dijkstra.
}

// Function joins those vertices of triangles which form valid path in our graph.
int joinVertices(struct triangle *t, int tri, struct edge *keep_edges)
{
    int n = 0,i = 0,j = 0,s = 0,f = 0;        //variable for loops
    int flag=0,r=0;         //variable to keep values of various checks
    int r1=0;
    n = tri ;   //number of triangles
    int N = (n * 3) ;//number of edges=number of vrtices
    struct point ver[N];
//Assign values to an array of points-'Ver'. Stores vertices of triangles.
    for(i = 0; i < n; i++)
    {
        ver[j].x = t[i].a.x, ver[j].y = t[i].a.y;
        ver[j+1].x = t[i].b.x, ver[j+1].y = t[i].b.y;
        ver[j+2].x = t[i].c.x, ver[j+2].y = t[i].c.y;
        j = j + 3;
    }
//check if any vertices in triangle
   for (i = 0; i < N; i++)  //loop runs for number of vertices times
    {
        int not_in = 1;
        for(j = 0; j < n; j++)  //loop runs for number of triangles times
        {
          if(((ver[i].x != t[j].a.x) && (ver[i].y != t[j].a.y))||
             ((ver[i].x != t[j].b.x) && (ver[i].y != t[j].b.y))||
             ((ver[i].x != t[j].c.x) && (ver[i].y != t[j].c.y)))
          {
            r1 = isPointInTriangle(ver[i],t[j]);
            if(r1) { not_in = 0; }
          }
        }
        if(!not_in) // If vertex is inside triangle, set it coordinates to infinty.
        {
        ver[i].x = infinity, ver[i].y = infinity;}
    }
  // Join vertices and keeps those vertices which has valid path */
    for(i = 0; i < N; i++) //loop works for number of vertices times
    {
        for(j = 0; j < N; j++)
        {
            if(i!=j)
            {
            flag = 1; //temporary variable for validating the path.
            for(s = 0; s < n; s++)          //loop runs for number of triangles
            {

            // Check if ver[i] is one the vertices of current triangle
                if((ver[i].x == t[s].a.x) && (ver[i].y == t[s].a.y))
                {
                    r = isIntersect(ver[i], ver[j], t[s].b, t[s].c);
                    if((r == 0) && (flag == 1))
                    flag = 1;
                    else
                    flag = 0;
                }
                else if((ver[i].x == t[s].b.x) && (ver[i].y == t[s].b.y))
                {
                    r = isIntersect(ver[i], ver[j], t[s].c, t[s].a);
                    if((r==0) &&(flag == 1))
                    flag = 1;
                    else
                    flag = 0;
                }
                else if((ver[i].x == t[s].c.x) && (ver[i].y == t[s].c.y))
                {
                    r = isIntersect(ver[i],ver[j],t[s].a,t[s].b);
                    if((r==0) && (flag == 1))
                    flag = 1;
                    else
                    flag = 0;
                }
            // Check if ver[j] is one the vertices of current triangle
                else if((ver[j].x == t[s].a.x) && (ver[j].y == t[s].a.y))
                {
                    r = isIntersect(ver[i],ver[j],t[s].b,t[s].c);
                    if((r==0) && (flag == 1))
                    flag = 1 ;
                    else
                    flag = 0 ;
                }
                else if((ver[j].x == t[s].b.x) && (ver[j].y == t[s].b.y))
                {
                    r = isIntersect(ver[i],ver[j],t[s].c,t[s].a);
                    if((r==0) && (flag==1))
                    flag = 1;
                    else
                    flag=0;
                }
                else if((ver[j].x == t[s].c.x) && (ver[j].y == t[s].c.y))
                {
                    r = isIntersect(ver[i],ver[j],t[s].a,t[s].b);
                    if ((r==0) && (flag==1))
                    flag = 1;
                    else
                    flag = 0;
                }
            // When vertices v[i],v[j] do not belong to current triangle t[s].
                else
                {
                    r = isIntersectEdge(ver[i],ver[j],t[s]);
                    if ((r == 1) && (flag==1))
                    flag = 1;
                    else
                    flag = 0;
                }

            }    // If the line is a valid line,keep the coordinates.
            if(flag==1)
            {
                if( ( (ver[i].x != infinity) && (ver[i].y != infinity) )
                  && ( (ver[j].x != infinity)&& (ver[j].y != infinity ) ) )
                {
                    //XDrawLine(display, win, gc_blue, ver[i].x, ver[i].y, ver[j].x, ver[j].y);
                    keep_edges[f].v1.x = ver[i].x , keep_edges[f].v1.y = ver[i].y;
                    keep_edges[f].v2.x = ver[j].x , keep_edges[f].v2.y = ver[j].y;
                    f++;
                }
            }

}
 }    }
return f; //returns number of valid paths.
}
// This function find shortest path between start and target point by applying dijkstra shortest path algorithm
void  applyDijkstra(struct triangle *t, struct edge *keep_edges,
                            struct point start, struct point target,
                            int number_of_triangles, int number_of_paths)
{
    int num_nodes = ( 3 * (number_of_triangles )) + 2;
    struct point nodes[num_nodes];
    int node_number = 0;
    int source = 0;
    int destination = 0;
    int cost[num_nodes][num_nodes];
    int i = 0, j = 0, k = 0;

    // Initialize the nodes array which stores start point, target point and all vertices of triangles.
    // First and second element of nodes array is start and target point.
    // After that it store vertices of triangles.
    nodes[node_number].x = start.x ,nodes[node_number].y = start.y, source = node_number;
    node_number++ ;
    nodes[node_number].x = target.x , nodes[node_number].y = target.y , destination=node_number ;
    node_number++;
    for(i = 0; i < (number_of_triangles) ; i++)
    {
        nodes[node_number].x = t[i].a.x, nodes[node_number].y = t[i].a.y;
        nodes[node_number + 1].x = t[i].b.x, nodes[node_number + 1].y = t[i].b.y;
        nodes[node_number + 2].x = t[i].c.x, nodes[node_number + 2].y = t[i].c.y;
        node_number = node_number + 3;
    }
//Set cost of all paths as infinity
    for(i = 0; i < num_nodes; i++)
    {
        for(j = 0; j < num_nodes; j++)
        cost[i][j] = infinity;
    }
//Get cost of each path
    for(i = 0; i < num_nodes; i++)
    {
        for(j = 0; j < num_nodes; j++)
        {
            for(k = 0; k < number_of_paths; k++)
            {
                if(((nodes[i].x == keep_edges[k].v1.x) && (nodes[i].y == keep_edges[k].v1.y))
                   &&((nodes[j].x == keep_edges[k].v2.x) && (nodes[j].y == keep_edges[k].v2.y)))
                {
                    //XDrawLine(display,win,gc_red,keep_edges[k].v1.x,keep_edges[k].v1.y,keep_edges[k].v2.x,keep_edges[k].v2.y);
                    cost[i][j] = cost[j][i] = calculateLength( keep_edges[k] );
                }
            }

        }
    }
// Apply dijkstra
    int distance[num_nodes], previous[number_of_paths], selected_node[num_nodes], source_point;
    struct point path[number_of_paths];
    int location,minimum,length;
    for(i = 0;i < num_nodes ;i++)
    {
        distance[i] = infinity;
        selected_node[i] = 0;
        previous[i] = -1;
    }
    source_point = source;
    selected_node[source_point] = 1;
    distance[source_point] = 0;

    while(selected_node[destination] ==0)
    {
        minimum = infinity;
        location = 0;
        for(i = 0;i< num_nodes;i++)
        {
            length = distance[source_point] + cost[source_point][i];
            if(length < distance[i] && selected_node[i]==0)
            {
                distance[i] = length;
                previous[i] = source_point;
            }
            if(minimum > distance[i] && selected_node[i]==0)
            {
                minimum = distance[i];
                location = i;
            }
        }
        source_point = location;
        selected_node[source_point] = 1;
    }
    // retrieve the nodes of shortest path and store in array of points-'Path'.
    source_point = destination;
    j=0; //for loop, temporary variable.
    while(source_point != -1)
    {

        path[j].x = nodes[source_point].x, path[j].y = nodes[source_point].y ;
        source_point = previous[source_point];
        ++j;
    }
    // Draws the shortest path from source to destination point.
    for(i=0;i<j;i++)
    {
        k=i+1;
        if(k!=j)
        XDrawLine(display,win,gc_blue,path[i].x,path[i].y,path[k].x,path[k].y);
    }
}
// This function calculate the euclidean length of given edge
int calculateLength(struct edge paths)
{
    int a1 = (paths.v1.x) - (paths.v2.x);
    int a2 = (paths.v1.y) - (paths.v2.y);
    a1 = a1 * a1;
    a2 = a2 * a2;
    int d= a1 + a2;
    int distance = sqrt( d );
    return distance;
}
