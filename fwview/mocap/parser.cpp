
/* 
   for 6 chan: (xt, yt, zt, zr, xr, yr);
   for 3 chan: (zr, xr, yr)
*/

#include "stdafx.h"
#include "parser.h"
#include "MotionCapturer.h"


// reading hierchary tree
bool init_node(char *hdr_lineBuf, Node* &node);
	// alloc mem for a node set type

bool fill_node(FILE *file, Node* node, int &rot_cnt, int &rot_off, int *nodenr);

/* fill each node's parent after the hierachy is built. */
void fill_parent(Node *root);

bool get_offsets(Node *node, char *line );	
	// read in offsets

bool get_channels(Node *node, char *line );// syntax checking, not relevant
	



////// for reading motin data portion
bool	fill_transform(FILE *file, MotionCapturer *cap);
	// alloc and fill in trans and e_rot array

bool	read_motion(FILE *file, MotionCapturer *cap);
	/* read in the motion part of data, 
		fills cap's frame_cnt, frame_time, and trans/rot data.
	*/

// reading hierchary tree
// alloc mem for a node and assign its type and name
bool init_node(char *hdr_lineBuf, Node* &node)
{
	char *tok;

  	tok = strtok(hdr_lineBuf, " \t");

	// alloc and init node
	node = (Node *)malloc( NODE_SIZE );
	memset(node, 0, NODE_SIZE);
	
	if ( strcmp(tok, "ROOT") == 0 )
		node->type = ROOT;
	else if ( strcmp(tok, "JOINT") == 0 )
		node->type = JOINT;
	else if ( strcmp(tok, "End") == 0) {
		tok = strtok(NULL, " \t\n");
		if ( strcmp(tok, "Site") != 0 ) return false;
			node->type = END_SITE;
	}
	else {
		free(node);
		return false;
	}

	// get root/joint's name
	if (node->type != END_SITE) {
		tok = strtok(NULL, " \t\n");
		memcpy(node->name, tok, strlen(tok)+1);
	}

  return true;

}


/* read in offset, chan, and children structural info for a node
	rot_off = 0, rot_cnt = 0 before first called, inc after each call 
	fills in the offset embedded in hierarchy,
	and get rot_cnt (NOTE: end_site has no rotation, doesn't count as a node)
*/
bool fill_node(FILE *file, Node* node, int &rot_cnt, int &rot_off, int *nodenr)
{
	bool status;
	int i;
	char hdr_lineBuf[LINESIZE];	// where to read the line into
	char line[LINESIZE];	// store a copy of hdr_lineBuf 

	char *tok;

  // skip "{"
  if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
  tok = strtok(hdr_lineBuf, " \t\n");
  if ( strcmp(tok, "{") != 0) return false;

  // get offset
  if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;	
  if ( get_offsets(node, hdr_lineBuf) != true) return false;
  
  if (node->type == END_SITE)
  {	// end_site only has offsets
	 // skip "}"
	if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
	tok = strtok(hdr_lineBuf, " \t\n");
	if ( strcmp(tok, "}") != 0) return false;

	return true;
  }
	
  // get chans
	if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
	if ( get_channels(node, hdr_lineBuf) != true) return false;


	// calc offset to index into the rotation data for this child
	node->e_off = rot_off * 3;
	node->nodeNr=*nodenr;
	*nodenr=*nodenr+1;
	rot_off++;
	rot_cnt++;

	// get children
	for (i=0; i<MAX_CHILDREN; i++) {

		// check if there are children 
		if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
			memcpy(line, hdr_lineBuf, strlen(hdr_lineBuf)+1);
		
		tok = strtok(hdr_lineBuf, " \t\n");
		if ( strcmp(tok, "}") == 0 ) {

			return true;		// "}" => done, no more children
		}
		else {	// else processing the child
			if ( init_node(line, node->children[i]) == false ) return false;

			status = fill_node(file, node->children[i], rot_cnt, rot_off, nodenr);	// recurse
			if (status == false) return false;
			node->children_cnt++;
		}

	}
		
	return true;
}


/* fill each node's parent after the hierachy is built. */
void fill_parent(Node *root)
{

	int i;

	if (root == NULL) return;

	for (i=0; i<root->children_cnt; i++) {
		root->children[i]->parent = root;
		fill_parent(root->children[i]);
	}

}



// read in offsets
bool get_offsets(Node *node, char *line )
{
  char *tok;

	tok = strtok(line, " \t");
	if ( strcmp(tok, "OFFSET") != 0 ) return false;
	
	tok = strtok(NULL, " \t");
	if (!tok) return false; 
		node->offset[0] = atof(tok);
	node->currentpos[0]=node->offset[0];

	tok = strtok(NULL, " \t");
	if (!tok) return false; 
		node->offset[1] = atof(tok);
		node->currentpos[1]=node->offset[1];

		tok = strtok(NULL, " \t");
	if (!tok) return false; 
	node->offset[2] = atof(tok);
	node->currentpos[2]=node->offset[2];

	return true;
}



bool read_bvh_file(char *filename, MotionCapturer *motionCapturer)
{
	FILE *file = NULL;
	char hdr_lineBuf[LINESIZE];	// where to read the line into
	char *tok;    // used to tokenize each line of the file.
	bool status;
	int nodenr=1;
	file = fopen(filename,"r");
	if (!file) {
	 
	    return false;
	}
	else
	
	if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
	tok = strtok(hdr_lineBuf, " \t\n");

	if( strcmp(tok, "HIERARCHY") != 0 )		return false;

	if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
	if ( init_node(hdr_lineBuf, motionCapturer->root) != true ) return false;
	// alloc a node and assign its type and its offset to index into the rotation data.
	motionCapturer->root->e_off = 0;	// offset to euler array


	motionCapturer->rot_cnt = 0;	
	int rot_off = 0;
		// rot offset keeps track of num nodes having rot off set, to be incr per recursion

	if ( fill_node(file, motionCapturer->root, motionCapturer->rot_cnt, rot_off, &nodenr) != true ) return false;	
	// recurse. fill node's name, offset, chan, children info.
	
	fill_parent(motionCapturer->root);
	motionCapturer->root->parent = NULL;


	status = read_motion(file, motionCapturer);
		
	fclose(file);
	return status;

}



/* read in the motion part of data, 
	fills cap's frame_cnt, frame_time, and trans/rot data.
*/
bool read_motion(FILE *file, MotionCapturer *cap)
{
	bool status;
	char hdr_lineBuf[LINESIZE];	// where to read the line into
	char *tok;    // used to tokenize each line of the file.

	// get keyword "MOTION"
	if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
	tok = strtok(hdr_lineBuf, " \t\n");
	if (tok == NULL) {	
		if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
		tok = strtok(hdr_lineBuf, " \t\n");
	}
	if( strcmp(tok, "MOTION") != 0 ) return false;

	// get keyword "Frames: #"
	if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
	tok = strtok(hdr_lineBuf, " \t");
	if( strcmp(tok, "Frames:") != 0 )		return false;
	tok = strtok(NULL, " \t\n");
	cap->frame_cnt = atoi(tok);			// get frame cnt

	// get keyword "Frame Time: #"
	if (!fgets(hdr_lineBuf, LINESIZE, file)) return false;
	tok = strtok(hdr_lineBuf, " \t");
	if( strcmp(tok, "Frame") != 0 )		return false;
	tok = strtok(NULL, " \t");
	if( strcmp(tok, "Time:") != 0 )		return false;
	tok = strtok(NULL, " \t\n");
	cap->frame_time = atof(tok);		// get frame time

	status = fill_transform(file, cap);


	return status;
}




/* fills cap's trans array (root's translate) and 
	euler array 
*/
bool fill_transform(FILE *file, MotionCapturer *cap)
{
	double *t_ptr;	// ptr to trans array
	double *e_ptr;	// ptr to euler array
	int i, j;
	int t_size	= 3 * cap->frame_cnt;	// TODO: frame_cnt * 3 * root_cnt
	int e_cnt_per_frame = cap->rot_cnt * 3;	// number of eulers per frame
	int e_size  = e_cnt_per_frame * cap->frame_cnt;	// 3 angles per euler rot per node


	cap->trans = NULL;
	cap->trans = (double*) malloc( t_size * sizeof(double));	
	if (cap->trans == NULL) return false;
	memset(cap->trans, 0, t_size);

	cap->euler = NULL;
	cap->euler = (double*) malloc( e_size * sizeof(double));
	if (cap->euler == NULL) return false;
	memset(cap->euler, 0, e_size);

	t_ptr = cap->trans;
	e_ptr = cap->euler;

	/* note: in case of unexpected EOF, i still try to continue,
		will try to draw fig w/ missing data */
	for (i = 0; i < cap->frame_cnt; i++) {

		// store first 3 as root's transl
		if (fscanf(file, "%lf", t_ptr++ ) != 1) {
			
				break;
		}
		if (fscanf(file, "%lf", t_ptr++ ) != 1) {
			
				break;
		}
		if (fscanf(file, "%lf", t_ptr++ ) != 1) {
			
				break;
		}
		
		// store each node's rot
		for (j=0; j < e_cnt_per_frame; j++) {
			
			if (fscanf(file, "%lf", e_ptr++) != 1) {
				fflush(stdout);
				break;
			}
		}

	}

	return true;
}
	


bool get_channels(Node *node, char *line )
{
	char *tok;

	tok = strtok(line, " \t");
	if ( strcmp(tok, "CHANNELS") != 0 ) return false;

	tok = strtok(NULL, " \t");
	if (!tok) return false; 
	node->chan_cnt = atoi(tok);
	if (node->chan_cnt != 3 && node->chan_cnt != 6) {
	  return false;
	}

	// below is to verify valid chan layout e.g. 3 transl chan are together
	tok = strtok(NULL, " \t");
	if (!tok) return false; 	  
	if ( strcmp(tok, "Xposition") == 0 ) {

	  node->b_first3trans = true; // check trans channels

	  tok = strtok(NULL, " \t");
	  if (!tok) return false; 	  
	  if ( strcmp(tok, "Yposition") != 0 ) return false;
	  
	  if (node->chan_cnt == 3)
		tok = strtok(NULL, " \t\n");
	  else
		tok = strtok(NULL, " \t");
	  if (!tok) return false; 	  
	  if ( strcmp(tok, "Zposition") != 0 ) return false;
	}
	else if (strcmp(tok, "Zrotation") == 0) {

	  node->b_first3trans = false; // check rot channels

	  tok = strtok(NULL, " \t");
	  if (!tok) return false; 	  
	  if ( strcmp(tok, "Xrotation") != 0 ) return false;
	  
	  if (node->chan_cnt == 3)
		tok = strtok(NULL, " \t\n");
	  else
		tok = strtok(NULL, " \t");
	  if (!tok) return false; 	  
	  if ( strcmp(tok, "Yrotation") != 0 ) return false;
	}
	else 
	  return false;

	if (node->chan_cnt == 6) { // check the next 3
	  if (node->b_first3trans == true) { // verfiy next 3 are rot
	    tok = strtok(NULL, " \t");
	    if (!tok) return false; 	  	    
		if ( strcmp(tok, "Zrotation") != 0 ) return false;
	    
	    tok = strtok(NULL, " \t");
	    if (!tok) return false; 	  
	    if ( strcmp(tok, "Xrotation") != 0 ) return false;
	  
	    tok = strtok(NULL, " \t\n");
	    if (!tok) return false;	
		if ( strcmp(tok, "Yrotation") != 0 ) return false;
	  }
	  else { // verify next 3 are trans
	    tok = strtok(NULL, " \t");
	    if (!tok) return false; 	  	    
	    if ( strcmp(tok, "Xposition") != 0 ) return false;

	    tok = strtok(NULL, " \t");
	    if (!tok) return false; 	  
	    if ( strcmp(tok, "Yposition") != 0 ) return false;

	    tok = strtok(NULL, " \t\n");
	    if (!tok) return false; 	  
	    if ( strcmp(tok, "Zposition") != 0 ) return false;
	  }
	}

	return true;
}






	
