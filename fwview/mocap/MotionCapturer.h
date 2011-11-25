
#ifndef MOTIONCAPTURER_H
#define MOTIONCAPTURER_H

#include <windows.h>		
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "parser.h"

// for bvh parser, assume MAX MAX_CHILDREN joints from a root

typedef struct Node Node;

#define MAX_CHILDREN	10	// max 10 child nodes (joints) from a parent (root)
#define NODE_SIZE		sizeof(Node)

//#//define ROOT	0
//#define	JOINT	1
//#define	END_SITE	2

  

struct Node {

  char	name[40];
  int	type;			//
						//	ROOT		= 0,
						//	JOINT		= 1,
						//	END_SITE	= 2
  double	currentpos[3];
  float	worldangle[3];
  double	offset[3];	// translation offset.  bad naming
  int		e_off;		// offset WITHIN A FRAME to the euler array in MotionCapturer. 

  int   	chan_cnt; // has to be 3 or 6
  int		nodeNr;
  bool		b_first3trans;
  int orientation;


  Node *children[MAX_CHILDREN];
  int		children_cnt;

  Node	*parent;	// parent joint.
  
 
 };



class MotionCapturer {

public:
	Node	*root;
	int		rot_cnt;	// number of nodes having rotations (root + joints)
	int		frame_cnt;	// number of frames in this anim
	double	frame_time;	// time in sec for each frame

	double	*trans;	
		/* arr of root's translation at each frame.
			arr size = 3 * root_cnt * frame_cnt, root_cnt is usually 1	
		*/

	double	*euler;		
		/* arr of euler rotations read from .bvh file.  each rotation angle in zxy order.
			and each rotation specified in hierachy order.  
			Also all rotations are specified for one frame before the next frame.
			arr size = 3 * node_cnt * frame_cnt.
		*/


//	bool MotionCapturer::scaleNodes(int scalefactor,Node *roott);
	bool MotionCapturer::setupXFramePose(int frameX, Node *roott, int *nodeNr);
//	bool MotionCapturer::scaleOffset(int scalefactor,Node *roott);
//	void MotionCapturer::RotateNode(int frameX, Node *roott, int nodeNr);
//	double* MotionCapturer::GetBackPos(Node *roott,double* backpos);
//	void MotionCapturer::RotateNodeByPivot(int frameX, Node *roott, int nodeNr,double* pivot);
	Node* MotionCapturer::FindNodeNr( Node *roott, char* nname);


	MotionCapturer::MotionCapturer()
	{
		root = NULL;            
		frame_cnt = 0;
	};

	~MotionCapturer();

};



//--------- parser functions -------------

bool read_bvh_file(char *filename, MotionCapturer *motionCapturer);
	/* *****API**** called in viewer.cpp. 
	read in file and fill a MotionCapturer object w/ hierarchical and motion data info
	return SUCCESS if read file ok
	FAILURE otherwise
	*/


//--------- end parser functions-----------

#endif