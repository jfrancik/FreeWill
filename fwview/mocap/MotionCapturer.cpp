#include "stdafx.h"
#include "MotionCapturer.h"

void del_node(Node* &node)
{
	int i;

	if (node != NULL) {
		for (i = 0; i < node->children_cnt; i++) {		
			del_node(node->children[i]);
		}

		free(node);
	}

	node = NULL;

}


MotionCapturer::~MotionCapturer()
{

	del_node(root);

	free(trans);

}
/*
bool MotionCapturer::scaleNodes(int scalefactor,Node *roott)
{
	
	for (int i=0; i<roott->children_cnt; i++) {
		roott->currentpos[0]=roott->currentpos[0]/scalefactor;
		roott->currentpos[1]=roott->currentpos[1]/scalefactor;
		roott->currentpos[2]=roott->currentpos[2]/scalefactor;
		scaleNodes(scalefactor, roott->children[i]);
		}
	
	return true;
}
double* MotionCapturer::GetBackPos(Node *roott,double* backpos)
{

 if(roott->type==0)
 {
	 backpos[0]=roott->currentpos[0];
	 backpos[1]=roott->currentpos[1];
	 backpos[2]=roott->currentpos[2];
 return backpos;
 }

 if(roott->parent->type==0)
 {
	 backpos[0]=backpos[0]+roott->parent->currentpos[0];
	 backpos[1]=backpos[1]+roott->parent->currentpos[1];
	 backpos[2]=backpos[2]+roott->parent->currentpos[2];
 return backpos;
 }

 if(roott->parent->type!=0)
 {
	 backpos[0]=backpos[0]+roott->currentpos[0];
	 backpos[1]=backpos[1]+roott->currentpos[1];
	 backpos[2]=backpos[2]+roott->currentpos[2];
 return GetBackPos(roott->parent,backpos);
 }

return backpos;
}
bool MotionCapturer::scaleOffset(int scalefactor,Node *roott)
{
	
	for (int i=0; i<roott->children_cnt; i++) {
		roott->offset[0]=roott->offset[0]/scalefactor;
		roott->offset[1]=roott->offset[1]/scalefactor;
		roott->offset[2]=roott->offset[2]/scalefactor;
		scaleOffset(scalefactor, roott->children[i]);
		}
	
	return true;
}
 */

Node* MotionCapturer::FindNodeNr( Node *roott, char* nname)
{
	if(strcmp(nname,"Pelvis")==0)
	{
		if(roott->type==0)
			return roott;
	}
	if(strcmp(nname,"LThigh")==0)
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==1) && (roott->children[i]->offset[0]>0))
				 return roott->children[i];
			 
		 }
	 }


	}
	if(strcmp(nname,"LCalf")==0) //L Knee
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==1) && (roott->children[i]->offset[0]>0) && roott->children[i]->children[0]->children_cnt==1)
				 return roott->children[i]->children[0];
			 
		 }
	 }
	}
	if(strcmp(nname,"LFoot")==0) // L Ankle
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==1) && (roott->children[i]->offset[0]>0) && (roott->children[i]->children[0]->children_cnt==1) && (roott->children[i]->children[0]->children[0]->children_cnt==1) && (roott->children[i]->children[0]->children[0]->children[0]->type==2) )
				 return roott->children[i]->children[0]->children[0];
			 
		 }
	 }
	}
	if(strcmp(nname,"RThigh")==0)
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==1) && (roott->children[i]->offset[0]<0))
				 return roott->children[i];
			 
		 }
	 }


	}
	if(strcmp(nname,"RCalf")==0) //Knee
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==1) && (roott->children[i]->offset[0]<0) && roott->children[i]->children[0]->children_cnt==1)
				 return roott->children[i]->children[0];
			 
		 }
	 }
	}
	if(strcmp(nname,"RFoot")==0) //Ankle
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==1) && (roott->children[i]->offset[0]<0) && (roott->children[i]->children[0]->children_cnt==1) && (roott->children[i]->children[0]->children[0]->children_cnt==1) && (roott->children[i]->children[0]->children[0]->children[0]->type==2) )
				 return roott->children[i]->children[0]->children[0];
			 
		 }
	 }
	}
	if(strcmp(nname,"Spine")==0) //Chest
	{
	if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			if(roott->children[i]->children_cnt==3) 
				 return roott->children[i];
		 }
	 }
	}
	if(strcmp(nname,"LClavicle")==0) //L collar
{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]>0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j];
					}
			 }
		 }
	 }
	}
	if(strcmp(nname,"LUpperArm")==0) //L 
{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]>0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j]->children[0];
					}
			 }
		 }
	 }
	}
if(strcmp(nname,"LForearm")==0) //L Elbow
{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]>0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j]->children[0]->children[0];
					}
			 }
		 }
	 }
	}

if(strcmp(nname,"LHand")==0) //L Wrist
{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]>0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j]->children[0]->children[0]->children[0];
					}
			 }
		 }
	 }
	}
if(strcmp(nname,"RClavicle")==0) //L collar
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]<0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j];
					}
			 }
		 }
	 }
	}

	if(strcmp(nname,"RUpperArm")==0) //L collar
{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]<0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j]->children[0];
					}
			 }
		 }
	 }
	}
if(strcmp(nname,"RForearm")==0) //L Elbow
{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]<0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j]->children[0]->children[0];
					}
			 }
		 }
	 }
	}
if(strcmp(nname,"RHand")==0) //L Wrist
{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if(roott->children[i]->children_cnt==3) 
			 {
				 for(int j=0; j<roott->children[i]->children_cnt; j++)
					{
					 if((roott->children[i]->children[j]->children_cnt==1) && (roott->children[i]->children[j]->offset[0]<0) &&
				 (roott->children[i]->children[j]->children[0]->children_cnt==1) && (roott->children[i]->children[j]->children[0]->children[0]->children_cnt==1)&&
				 (roott->children[i]->children[j]->children[0]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[j]->children[0]->children[0]->children[0];
					}
			 }
		 }
	 }
	}
if(strcmp(nname,"Head")==0) //Head
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==3) && (roott->children[i]->children[0]->children_cnt==1) &&
				 (roott->children[i]->children[0]->children[0]->children_cnt==1) &&
				 (roott->children[i]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[0]->children[0];
		 }
	 }
	}

if(strcmp(nname,"Neck")==0) //Head
	{
	 if(roott->type==0)
	 {
		 for(int i=0; i<roott->children_cnt; i++)
		 {
			 if((roott->children[i]->children_cnt==3) && (roott->children[i]->children[0]->children_cnt==1) &&
				 (roott->children[i]->children[0]->children[0]->children_cnt==1) &&
				 (roott->children[i]->children[0]->children[0]->children[0]->type==2))
				 return roott->children[i]->children[0];
		 }
	 }
	}



	return  roott;
}
bool MotionCapturer::setupXFramePose(int frameX, Node *roott, int *nodeNr)
{
int i;
	for (i=0; i<roott->children_cnt; i++) {

		roott->currentpos[0]=roott->offset[0]+trans[frameX*3];
		roott->currentpos[1]=roott->offset[1]+trans[(frameX*3)+1];
		roott->currentpos[2]=roott->offset[2]+trans[(frameX*3)+2];

		*nodeNr=*nodeNr+1;

		setupXFramePose(frameX, roott->children[i],nodeNr);
		}
	
	return true;
}
/*
void MotionCapturer::RotateNode(int frameX, Node *roott, int nodeNr)
{

	double tmppos[3];
	

		if(roott->type!=0)//set up pivot in (0,0,0)
		{
			roott->currentpos[0]=roott->currentpos[0]-roott->parent->currentpos[0];
			roott->currentpos[1]=roott->currentpos[1]-roott->parent->currentpos[1];
			roott->currentpos[2]=roott->currentpos[2]-roott->parent->currentpos[2];
		}
// Y
		tmppos[0]=(roott->currentpos[0]*cos((euler[nodeNr*((frameX*3)+2)])*PI/180)) + (roott->currentpos[2]*sin((euler[nodeNr*((frameX*3)+2)])*PI/180));
		tmppos[1]=roott->currentpos[1];
		tmppos[2]=(roott->currentpos[2]*cos((euler[nodeNr*((frameX*3)+2)])*PI/180)) - (roott->currentpos[0]*sin((euler[nodeNr*((frameX*3)+2)])*PI/180));
		roott->currentpos[0]=tmppos[0];
		roott->currentpos[1]=tmppos[1];
		roott->currentpos[2]=tmppos[2];
// X
		tmppos[0]=roott->currentpos[0];
		tmppos[1]=(roott->currentpos[1]*cos((euler[nodeNr*((frameX*3)+1)])*PI/180)) - (roott->currentpos[2]*sin((euler[nodeNr*((frameX*3)+1)])*PI/180));
		tmppos[2]=(roott->currentpos[1]*sin((euler[nodeNr*((frameX*3)+1)])*PI/180)) + (roott->currentpos[2]*cos((euler[nodeNr*((frameX*3)+1)])*PI/180));
		roott->currentpos[0]=tmppos[0];
		roott->currentpos[1]=tmppos[1];
		roott->currentpos[2]=tmppos[2];

// Z	euler
		tmppos[0]=(roott->currentpos[0]*cos((euler[nodeNr*(frameX*3)])*PI/180)) - (roott->currentpos[1]*sin((euler[nodeNr*(frameX*3)])*PI/180));
		tmppos[1]=(roott->currentpos[0]*sin((euler[nodeNr*(frameX*3)])*PI/180)) + (roott->currentpos[1]*cos((euler[nodeNr*(frameX*3)])*PI/180));
		tmppos[2]=roott->currentpos[2];
		roott->currentpos[0]=tmppos[0];
		roott->currentpos[1]=tmppos[1];
		roott->currentpos[2]=tmppos[2];



		if(roott->type!=0)
		{		
			roott->currentpos[0]=roott->currentpos[0]+roott->parent->currentpos[0];
			roott->currentpos[1]=roott->currentpos[1]+roott->parent->currentpos[1];
			roott->currentpos[2]=roott->currentpos[2]+roott->parent->currentpos[2];
			for (int i=0; i<roott->children_cnt; i++)
			{
				RotateNodeByPivot(frameX, roott->children[i],nodeNr,roott->parent->currentpos);
			}
		}else
		{
			if(roott->type==0)
			{
				for (int i=0; i<roott->children_cnt; i++)
				{
					RotateNodeByPivot(frameX, roott->children[i],nodeNr,roott->currentpos);
				}
			}

		}
return;
}

void MotionCapturer::RotateNodeByPivot(int frameX, Node *roott, int nodeNr,double* pivot)
{

	double tmppos[3];

	roott->currentpos[0]=roott->currentpos[0]-pivot[0];
	roott->currentpos[1]=roott->currentpos[1]-pivot[1];
	roott->currentpos[2]=roott->currentpos[2]-pivot[2];
	
// Z	euler
	tmppos[0]=(roott->currentpos[0]*cos((euler[nodeNr*(frameX*3)])*PI/180)) - (roott->currentpos[1]*sin((euler[nodeNr*(frameX*3)])*PI/180));
	tmppos[1]=(roott->currentpos[0]*sin((euler[nodeNr*(frameX*3)])*PI/180)) + (roott->currentpos[1]*cos((euler[nodeNr*(frameX*3)])*PI/180));
	tmppos[2]=roott->currentpos[2];

	roott->currentpos[0]=tmppos[0];
	roott->currentpos[1]=tmppos[1];
	roott->currentpos[2]=tmppos[2];
// X
	tmppos[0]=roott->currentpos[0];
	tmppos[1]=(roott->currentpos[1]*cos((euler[nodeNr*((frameX*3)+1)])*PI/180)) - (roott->currentpos[2]*sin((euler[nodeNr*((frameX*3)+1)])*PI/180));
	tmppos[2]=(roott->currentpos[1]*sin((euler[nodeNr*((frameX*3)+1)])*PI/180)) + (roott->currentpos[2]*cos((euler[nodeNr*((frameX*3)+1)])*PI/180));
	roott->currentpos[0]=tmppos[0];
	roott->currentpos[1]=tmppos[1];
	roott->currentpos[2]=tmppos[2];
// Y
	tmppos[0]=(roott->currentpos[0]*cos((euler[nodeNr*((frameX*3)+2)])*PI/180)) + (roott->currentpos[2]*sin((euler[nodeNr*((frameX*3)+2)])*PI/180));
	tmppos[1]=roott->currentpos[1];
	tmppos[2]=(roott->currentpos[2]*cos((euler[nodeNr*((frameX*3)+2)])*PI/180)) - (roott->currentpos[0]*sin((euler[nodeNr*((frameX*3)+2)])*PI/180));
	roott->currentpos[0]=tmppos[0];
	roott->currentpos[1]=tmppos[1];
	roott->currentpos[2]=tmppos[2];
	if(roott->type!=0)
	{		
	roott->currentpos[0]=roott->currentpos[0]+pivot[0];
	roott->currentpos[1]=roott->currentpos[1]+pivot[1];
	roott->currentpos[2]=roott->currentpos[2]+pivot[2];
	}
		for (int i=0; i<roott->children_cnt; i++) 
		{
		RotateNodeByPivot(frameX, roott->children[i],nodeNr, pivot);
		}
return;
}*/