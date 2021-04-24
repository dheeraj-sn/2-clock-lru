#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include<stdlib.h>
#include <stdbool.h>
#include<iostream>
#include<list>
#include<utility>
#include<map>
#include<vector>
#include<math.h>
#include<set>
#include<string>
using namespace std;

// structure for each LRU node
// It stores seqno, num and the allocated frame_number
struct lru_node{
	int seqno;
    int num;
    int frame_number;
};

// Doubly linked list for active list
list<lru_node>active;

// Doubly linked list for inactive list
list<lru_node>inactive;

// Pool of free memory frames
set<int>free_memory;

//Hash table for tracking seqno, num and allocated frame number => {seqno, {num, frame_number}}
map<int,vector<pair<int,int> > >mem_map;

// Function to print the free memory frames
void print_free_memory(){
    for(auto it:free_memory)
        cout<<it<<" ";
    cout<<endl;
}

// Function to print the active list
void print_active(){
    for(auto it:active){
        cout<<"{"<<"seqno:"<<it.seqno<<", num: "<<it.num<<", frame_no: "<<it.frame_number<<"} "<<endl;
    }
    //cout<<endl;
}

// Function to print the inactive list
void print_inactive(){
    for(auto it:inactive){
        cout<<"{"<<"seqno:"<<it.seqno<<" ,num: "<<it.num<<", frame_no: "<<it.frame_number<<"} "<<endl;
    }
    //cout<<endl;
}

// Function to print the hash table which keeps track of seqno, num and allocated frame_number
// {seqno, {num, frame_number}}
void view_memory_map(){
    map<int,vector<pair<int,int> > >::iterator it;
    for(it=mem_map.begin();it!=mem_map.end();it++){
        cout<<it->first<<" -> ";
        vector<pair<int,int> >::iterator j;
        for(j=(it->second).begin();j!=(it->second).end();j++){
            cout<<"{"<<j->first<<","<<j->second<<"} ";
        }
        cout<<endl;
    }
}

// Function for removing a node from the tail of inactive list
void remove_from_inactive(){
    if(inactive.empty()){
        //If the inactive list is empty, return
        return ;
    }
    else{
        // Get the seqno, num and frame_number of the tail node in inactive list
		int seqno_current = inactive.back().seqno;
		int num_current = inactive.back().num;
		int frame_number_current = inactive.back().frame_number;

        // Find the tail node in the hash_table[seqno]
		for(int i=0;i<mem_map[seqno_current].size();i++){
            // Find the "num" page in "seqno"
			if(mem_map[seqno_current][i].first == num_current){
			    // Add the frame_number allocated to page "seqno, num" to the free memory frame pool
				free_memory.insert(mem_map[seqno_current][i].second);
				// Remove the page "seqno, num" from the hashtable
				mem_map[seqno_current].erase(mem_map[seqno_current].begin() + i);
				break;
			}
		}
		// Remove node from the tail of inactive list
        inactive.pop_back();
    }
}

// Function to add a node to the head of inactive list
void add_to_inactive(lru_node newNode){
    // Check if inactive list is full
    if(inactive.size()>=250){
        // If inactive list is full, call function to remove a node from the tail of the list
        remove_from_inactive();
    }
    // Push the new node(seqno, num, frame_number) to the head of the inactive list
    inactive.push_front(newNode);
}

// Function to remove a node from the tail of active list and add to head of inactive list
void remove_from_active(){
    // Add the tail node of active list to inactive list
    add_to_inactive(active.back());

    // Remove the tail node of active list
    active.pop_back();
}

// Function to add a node(seqno, num, frame_number) to head of active list
void add_to_active(lru_node newNode){
    // Check if active list is full
    if(active.size()>=250){
        // If active list is full, remove a node from the tail of active list and insert into the inactive list
        remove_from_active();
    }

    // Push the new node(seqno, num, frame_number) to the head of active list
    active.push_front(newNode);
}

// Function to reclaim allocated frames. It takes the number of frames required to be reclaimed as input
void reclaim(int number_of_frames){

    // Start removing nodes from the tail of inactive list
    // Keep removing frames till either the number of required frames have been reclaimed or the inactive list is empty
    while(number_of_frames!=0 && !inactive.empty()){
        remove_from_inactive();
        number_of_frames--;
    }

    // If the required number of frames have been reclaimed then return
    if(number_of_frames==0){
        return;
    }

    // If required number of frames have not been reclaimed from the inactive list, start removing nodes from tail of active list
    // Keep removing frames till either the number of required frames have been reclaimed or the active list is empty
    while(number_of_frames!=0 && !active.empty()){
        remove_from_active();
        remove_from_inactive();
        number_of_frames--;
    }
    return ;
}
///////////////////////////////////////////////////////////////////

// Function to initialize the memory pool
// For our requirement there is a memory of 2MB, i.e. 512 frames
void init(int pages){
	for(int i=0;i<pages;i++){
		free_memory.insert(i);
	}
}

// Function to allocate "num" number of frames for sequence number "seqno"
void allocate_frames(int seqno,int num){
    // Check if the "seqno" is already present in hash_table
	if(mem_map.find(seqno)==mem_map.end()){
		// If "seqno" is not present in hash_table, add a empty vector corresponding to "seqno" in hash_table
		mem_map[seqno]=vector<pair<int,int> >();
	}
	// Check if required number of frames are present in the memory pool
	if(num > free_memory.size()){
        // If required number of frames are not present, check the number of frames that need to be reclaimed for the allocation
		int frames_required = num - free_memory.size();

		// Reclaim the required number of frames
		reclaim(frames_required);
	}

    // Allocate "num" number of frames for sequence number "seqno"
	for(int i=0;i<num;i++){
        // Get frame number of free frame
		int get_page = *free_memory.begin();

        // Remove the free frame from the memory pool
		free_memory.erase(free_memory.begin());

		// Add (seqno, {num, frame_number}) to hash_table
		mem_map[seqno].push_back({i,get_page});

		// Prepare the new node for adding to inactive list
		lru_node newNode;
        newNode.seqno = seqno;
        newNode.num = i;
        newNode.frame_number = get_page;

        //  Add the new node {seqno, num, frame_number} to head of inactive list
        add_to_inactive(newNode);
	}
}

// Function to allocate a single frame for "seqno" and "num"
int allocate_one_frame(int seqno, int num){
    // Check if the "seqno" is already present in hash_table
	if(mem_map.find(seqno)==mem_map.end()){
        // If "seqno" is not present in hash_table, add a empty vector corresponding to "seqno" in hash_table
		mem_map[seqno]=vector<pair<int,int> >();
	}

    /*
	// Check if frame is already allocated for "seqno, num"
    for(int i=0;i<mem_map[seqno].size();i++){
        if(mem_map[seqno][i].first==num){
            // If frame is already allocated return
            return -1;
        }
    }
    */

    // Check if required number of frames are present in the memory pool
	if(num > free_memory.size()){
        // If required number of frames are not present, check the number of frames that need to be reclaimed for the allocation
		int frames_required = num - free_memory.size();
        // Reclaim the required number of frames
		reclaim(frames_required);
	}

	// Get frame number of free frame
	int get_page = *free_memory.begin();

	// Remove the free frame from the memory pool
	free_memory.erase(free_memory.begin());

	// Add (seqno, {num, frame_number}) to hash_table
	mem_map[seqno].push_back({num,get_page});

	// Prepare the new node for adding to inactive list
	lru_node newNode;
    newNode.seqno = seqno;
    newNode.num = num;
    newNode.frame_number = get_page;

    //  Add the new node {seqno, num, frame_number} to head of inactive list
    add_to_inactive(newNode);

    // Return the allocated frame number
	return get_page;
}

// Function to access page with "seqno, num"
void access_page(int seqno,int num){
	int found=false;
	list<lru_node>::iterator ait;

	// Check if allocated frame is present in active list
	for(ait=active.begin();ait!=active.end();ait++){
		if(ait->seqno == seqno && ait->num==num){
            // If allocated frame is present in active list, nothing needs to be done, return
			return;
		}
	}

	list<lru_node>::iterator iait;
	// Earlier if frame was not present in active list, flow will reach here
	// Check if frame is present in inactive list
	for(iait=inactive.begin();iait!=inactive.end();iait++){
		if(iait->seqno == seqno && iait->num==num){
            // If frame is present in inactive list it should be added to active list

            // Prepare node for adding to active list
			lru_node newNode;
			newNode.seqno = iait->seqno;
			newNode.num = iait->num;
			newNode.frame_number = iait->frame_number;

			// Add prepared node to active list
			add_to_active(newNode);

			// Remove node from inactive list
			inactive.erase(iait);
			return;
		}
	}

	// Flow will reach here if the frame is not present in either active or inactive list
	// This means that frame has not been allocated for this "seqno, num" of it has been reclaimed
	int allocated_frame_number = -1;
	// Check if "seqno" is present int our hash_table
	if(mem_map.find(seqno)==mem_map.end()){
        // If seqno is not present in hash_table then add a empty vector corresponding to seqno in hash_table
		mem_map[seqno]=vector<pair<int,int> >();
        // Allocate a frame for "seqno, num" and add to inactive list
        allocated_frame_number = allocate_one_frame(seqno, num);
        return;
	}

	// Check if frame is already allocated for "seqno, num"
	bool found_in_seq=false;
	for(int i=0;i<mem_map[seqno].size();i++){
		if(mem_map[seqno][i].first == num){
			allocated_frame_number = mem_map[seqno][i].second;
			found_in_seq = true;
			break;
		}
	}

	// If frame is not allocated for "seqno, num" then allocate a new frame for it and add to inactive list
	if(found_in_seq==false){
        allocated_frame_number = allocate_one_frame(seqno, num);
        return;
	}
	/*
	lru_node newNode;
    newNode.seqno = seqno;
    newNode.num = num;
    newNode.frame_number = allocated_frame_number;
    add_to_inactive(newNode);
    */
}

// Free the frame for "seqno, num"
void free_page(int seqno, int num){
	bool found = false;
	// Check if seqno is present in hash_table
	if(mem_map.find(seqno)==mem_map.end()){
		// If seqno is not present in hash_table then there is no frame allocated corresponding to it, so return
		return;
	}

	// Check if frame is present for "seqno, num" in our hash_table
	for(int i=0;i<mem_map[seqno].size();i++){
		if(mem_map[seqno][i].first == num){
            // If frame is present for "seqno, num" then reclaim the frame and add it to pool of free memory
			free_memory.insert(mem_map[seqno][i].second);
            // Remove the node for {seqno, {num,frame_numer}} from hash_table
			mem_map[seqno].erase(mem_map[seqno].begin() + i);
			found=true;
			break;
		}
	}
	// If frame was found in the hash_table check the active list and inactive list
	if(found){
		list<lru_node>::iterator ait;
		// Check if {seqno, num, frame_numer} is present in active list
		for(ait=active.begin();ait!=active.end();ait++){
			if(ait->seqno == seqno && ait->num==num){
                // Remove from active list
				active.erase(ait);
				break;
			}
		}
		list<lru_node>::iterator iait;
		// Check if {seqno, num, frame_numer} is present in inactive list
		for(iait=inactive.begin();iait!=inactive.end();iait++){
			if(iait->seqno == seqno && iait->num==num){
                // Remove from inactive list
				inactive.erase(iait);
				break;
			}
		}
	}
}

// Function to find the power of 2 just below a given number
int power_two(int n)
{
    int res = 0;
    for (int i=n; i>=1; i--)
    {
        if ((i & (i-1)) == 0)
        {
            res = i;
            break;
        }
    }
    return res;
}

// Function to make buddy lists from free frames
void make_buddy_list(){
    // Data structure for storing buddy lists
    map<int,vector<vector<int> > >mp;
    // Entry for storing blocks of size 1
    mp[1] = vector<vector<int> >();
    // Entry for storing blocks of size 2
    mp[2] = vector<vector<int> >();
    // Entry for storing blocks of size 4
    mp[4] = vector<vector<int> >();
    // Entry for storing blocks of size 8
    mp[8] = vector<vector<int> >();
    // Entry for storing blocks of size 16
    mp[16] = vector<vector<int> >();
    // Entry for storing blocks of size 32
    mp[32] = vector<vector<int> >();
    // Entry for storing blocks of size 64
    mp[64] = vector<vector<int> >();
    // Entry for storing blocks of size 128
    mp[128] = vector<vector<int> >();
    // Entry for storing blocks of size 256
    mp[256] = vector<vector<int> >();
    // Entry for storing blocks of size 512
    mp[512] = vector<vector<int> >();
    vector<int>free_temp;
    set<int>::iterator it;

    // Get list of all free frames from the memory pool
    for(it=free_memory.begin();it!=free_memory.end();it++){
        free_temp.push_back(*it);
    }
    int i=0;
    // Combine free memory frames into contiguous blocks of size equal to power of 2
    while(i<free_temp.size()){
        int j=i;
        // Checking the number of free frames which are contiguous starting from index i in the list of free frames
        while(j+1<free_temp.size() && (free_temp[j]==(free_temp[j+1]-1)) )
            j++;
        // There are (j - i + 1) free frames which are contiguous.
        // Get the power of 2 which is just less than or equal to (j-i+1) and store it in variable "res"
        int res = power_two(j-i+1);
        // Store contiguous frames from index i to index (i+res) in a temporary vector
        vector<int>temp;
        for(int k=i;k<i+res;k++){
            temp.push_back(free_temp[k]);
        }
        // Store the obtained block in map entry with index "res" i.e. size of the block which is power of 2
        mp[temp.size()].push_back(temp);
        // Increment i to i+res, to check for next contiguous block
        i=i+res;
    }

    // Print out the buddy lists
    map<int,vector<vector<int> > >::iterator mit;
    for(mit=mp.begin();mit!=mp.end();mit++){
        vector<vector<int> >::iterator vt;
        cout<<mit->first<<" -> ";
        for(vt=(mit->second).begin();vt!=(mit->second).end();vt++){
            cout<<"{";
            for(int j=0;j<(*vt).size()-1;j++){
                cout<<(*vt)[j]<<", ";
            }
            cout<<(*vt)[(*vt).size()-1];
            cout<<"}  ";
        }
        cout<<endl;
    }
}



///////////////////////////////////////////////////////////////////

int main()
{
	FILE *file = fopen( "A0225404R-assign4-input.dat", "r" );
	int arr[5000][2];
	char arrc[5000];
	char buffer[40];
	char temp[10];
	int i =0;
	int j =0;
	if( !file ) {
		puts("File not present error. Reformat the file name as : A0225404R-assign4-input.dat");
		return 0;
	}
	// Counter for length of input
	int inputLength = 0;
	// Read from input file
	while (!feof(file))
	{
	    // Read line
		fgets(buffer, 40, file);
		//cout<<buffer<<endl;
		// Get A/X/F
		arrc[inputLength]=buffer[0];


		// GET <seqno>
		i =2;
		j =0;
		while(buffer[i]!='\t'){
			temp[j]=buffer[i];
			i++;
			j++;
		}
		temp[j]='\0';
		arr[inputLength][0] = atoi(temp);


		// GET <num>
		i++;
		j=0;
		while(buffer[i]!='\n'){
			temp[j]=buffer[i];
			j++;
			i++;
		}
		temp[j]='\0';
		arr[inputLength][1] = atoi(temp);

		// Increment counter for input length
		inputLength++;
		//////////////////////////
	}

	// Close file
	fclose(file);

	// Initialize memory pool with 512 frames since memory size is 2MB
	init(512);


	for(i=0;i<inputLength;i++){
		// Processing for A <seqno> <num>
		if(arrc[i]=='A'){
            allocate_frames(arr[i][0],arr[i][1]);
		}

		// Processing for F <seqno> <num>
		else if(arrc[i]=='F'){
			free_page(arr[i][0],arr[i][1]);
		}

		// Processing for X <seqno> <num>
		else if(arrc[i]=='X'){
			access_page(arr[i][0],arr[i][1]);
		}
	}
    //view_memory_map();
    cout<<"========================================================================"<<endl;
    cout<<"Active list : \n================================"<<endl;
    cout<<"Printing format : {seqno, num, frame_number}"<<endl;
    cout<<"Active list HEAD ================================"<<endl;
    print_active();
    cout<<"Active list TAIL ================================"<<endl;
    cout<<"========================================================================"<<endl;
    cout<<"========================================================================"<<endl;
    cout<<"========================================================================"<<endl;
    cout<<"Inactive list : \n================================"<<endl;
    cout<<"Printing format : {seqno, num, frame_number}"<<endl;
    cout<<"Inactive list HEAD ================================"<<endl;
    print_inactive();
    cout<<"Inactive list TAIL ================================"<<endl;
    cout<<"========================================================================"<<endl;
    cout<<"========================================================================"<<endl;
    cout<<"========================================================================"<<endl;
    cout<<"Free frames : \n================================"<<endl;
    print_free_memory();
    cout<<"========================================================================"<<endl;
    cout<<"Number of frames in active list  : "<<active.size()<<endl;
    cout<<"Number of frames in inactive list : "<<inactive.size()<<endl;
    cout<<"Number of frames in the free pool : "<<free_memory.size()<<endl;
    cout<<"========================================================================"<<endl;
    cout<<"Status of buddy lists : \n================================"<<endl;
    cout<<"Block size -> {frames in 1st block} {frames in 2nd block} ..."<<endl;
    make_buddy_list();
    cout<<"========================================================================"<<endl;
	return 0;
}
