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
using namespace std;

struct lru_node{
	int seqno;
    int num;
    int frame_number;
};

list<lru_node>active;
list<lru_node>inactive;
set<int>free_memory;
map<int,vector<pair<int,int> > >mem_map;

void print_free_memory(){
    cout<<"Free frames : "<<endl;
    for(auto it:free_memory)
        cout<<it<<" ";
    cout<<endl;
}

void print_active(){
    cout<<"Active list : "<<endl;
    for(auto it:active){
        cout<<"{"<<"seqno:"<<it.seqno<<", num: "<<it.num<<", frame_no: "<<it.frame_number<<"} "<<endl;
        //cout<<"{"<<it.seqno<<","<<it.num<<","<<it.frame_number<<"} ";
    }
    cout<<endl;
}

void print_inactive(){
    cout<<"Inactive list : "<<endl;
    for(auto it:inactive){
        cout<<"{"<<"seqno:"<<it.seqno<<" ,num: "<<it.num<<", frame_no: "<<it.frame_number<<"} "<<endl;
        //cout<<"{"<<it.seqno<<","<<it.num<<","<<it.frame_number<<"} ";
    }
    cout<<endl;
}

void view_memory(){
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

int remove_from_inactive(){
    if(inactive.empty()){
        //cout<<"No frames in inactive list"<<endl;
        return 0;
    }
    else{
		int seqno_current = inactive.back().seqno;
		int num_current = inactive.back().num;
		int frame_number_current = inactive.back().frame_number;
		for(int i=0;i<mem_map[seqno_current].size();i++){
			if(mem_map[seqno_current][i].first == num_current){
				free_memory.insert(mem_map[seqno_current][i].second);
				//cout<<"Remove from mem_map : "<<seqno_current<<" "<<(mem_map[seqno_current].begin() + i)->first<<" "<<(mem_map[seqno_current].begin() + i)->second<<endl;
				mem_map[seqno_current].erase(mem_map[seqno_current].begin() + i);
				break;
			}
		}
		//cout<<"Remove from inactive : "<<inactive.back().seqno<<" "<<inactive.back().num<<" "<<inactive.back().frame_number<<endl;
        inactive.pop_back();
    }
    return 1;
}

int add_to_inactive(lru_node newNode){
    if(inactive.size()>=250){
        remove_from_inactive();
    }
    inactive.push_front(newNode);
    //cout<<"Add to inactive : "<<inactive.front().seqno<<" "<<inactive.front().num<<" "<<inactive.front().frame_number<<endl;
    return 1;
}

int remove_from_active(){
    //cout<<"Remove from active : "<<active.back().seqno<<" "<<active.back().num<<" "<<active.back().frame_number<<endl;
    add_to_inactive(active.back());
    active.pop_back();
    return 1;
}

int add_to_active(lru_node newNode){
    if(active.size()>=250){
        remove_from_active();
    }
    active.push_front(newNode);
    //cout<<"Add to active : "<<active.front().seqno<<" "<<active.front().num<<" "<<active.front().frame_number<<endl;
    return 1;
}

int reclaim(int number_of_frames){
    //cout<<"Mem size before reclaimation : "<<free_memory.size()<<endl;
    //cout<<"Try reclaim from inactive"<<endl;;
    while(number_of_frames!=0 && !inactive.empty()){
        remove_from_inactive();
        number_of_frames--;
    }
    if(number_of_frames==0){
        //cout<<"Mem size after reclaim from inactive: "<<free_memory.size()<<endl;
        return 1;
    }
    //cout<<"Try reclaim from active"<<endl;
    while(number_of_frames!=0 && !active.empty()){
        remove_from_active();
        remove_from_inactive();
        number_of_frames--;
    }
    //cout<<"Mem size after reclaimation : "<<free_memory.size()<<endl;
    return 1;
}
///////////////////////////////////////////////////////////////////


void init(int pages){
	for(int i=0;i<pages;i++){
		free_memory.insert(i);
	}
}


void allocate_pages(int seqno,int num){
    //cout<<"Memsize before allocate : "<<free_memory.size()<<endl;
	if(mem_map.find(seqno)==mem_map.end())
		mem_map[seqno]=vector<pair<int,int> >();
	if(num > free_memory.size()){
		int pages_required = num - free_memory.size();
		//cout<<"Try to reclaim "<<pages_required<<" pages."<<endl;
		reclaim(pages_required);
	}
	for(int i=0;i<num;i++){
		int get_page = *free_memory.begin();
		free_memory.erase(free_memory.begin());
		mem_map[seqno].push_back({i,get_page});
		lru_node newNode;
        newNode.seqno = seqno;
        newNode.num = i;
        newNode.frame_number = get_page;
        add_to_inactive(newNode);
	}
	//cout<<"Memsize after allocate : "<<free_memory.size()<<endl;
}

int allocate_one_page(int seqno, int num){
    //cout<<"Memsize before allocate 1 page : "<<free_memory.size()<<endl;
	if(mem_map.find(seqno)==mem_map.end())
		mem_map[seqno]=vector<pair<int,int> >();
    for(int i=0;i<mem_map[seqno].size();i++){
        if(mem_map[seqno][i].first==num)
            return 0;
    }
	if(num > free_memory.size()){
		int pages_required = num - free_memory.size();
		reclaim(pages_required);
	}
	int get_page = *free_memory.begin();
	free_memory.erase(free_memory.begin());
	mem_map[seqno].push_back({num,get_page});
	lru_node newNode;
    newNode.seqno = seqno;
    newNode.num = num;
    newNode.frame_number = get_page;
    add_to_inactive(newNode);
	//cout<<"Memsize after allocate 1 page : "<<free_memory.size()<<endl;
	return get_page;
}

void access_page(int seqno,int num){
	int found=false;
	list<lru_node>::iterator ait;
	for(ait=active.begin();ait!=active.end();ait++){
		if(ait->seqno == seqno && ait->num==num){
            //cout<<"Found seqno = "<<ait->seqno<<", num = "<<ait->num<<", frame_number = "<<ait->frame_number<<", in active list."<<endl;
			return;
		}
	}

	list<lru_node>::iterator iait;
	for(iait=inactive.begin();iait!=inactive.end();iait++){
		if(iait->seqno == seqno && iait->num==num){
            //cout<<"Found seqno = "<<iait->seqno<<", num = "<<iait->num<<", frame_number = "<<iait->frame_number<<", in inactive list."<<endl;
			lru_node newNode;
			newNode.seqno = iait->seqno;
			newNode.num = iait->num;
			newNode.frame_number = iait->frame_number;
			add_to_active(newNode);
			inactive.erase(iait);
			return;
		}
	}
	int allocated_frame_number = -1;
	if(mem_map.find(seqno)==mem_map.end()){
		mem_map[seqno]=vector<pair<int,int> >();
        allocated_frame_number = allocate_one_page(seqno, num);
        return;
	}
	bool found_in_seq=false;
	for(int i=0;i<mem_map[seqno].size();i++){
		if(mem_map[seqno][i].first == num){
			allocated_frame_number = mem_map[seqno][i].second;
			found_in_seq = true;
			break;
		}
	}
	if(found_in_seq==false){
        allocated_frame_number = allocate_one_page(seqno, num);
        return;
	}
	lru_node newNode;
    newNode.seqno = seqno;
    newNode.num = num;
    newNode.frame_number = allocated_frame_number;
    add_to_inactive(newNode);
}

void free_page(int seqno, int num){
    //cout<<"Memsize before free : "<<free_memory.size()<<endl;
	bool found = false;
	if(mem_map.find(seqno)==mem_map.end())
		return;
	for(int i=0;i<mem_map[seqno].size();i++){
		if(mem_map[seqno][i].first == num){
			free_memory.insert(mem_map[seqno][i].second);
			mem_map[seqno].erase(mem_map[seqno].begin() + i);
			found=true;
			//cout<<"Freed seqno = "<<seqno<<", num = "<<num<<endl;
			break;
		}
	}
	//if(found){
		list<lru_node>::iterator ait;
		for(ait=active.begin();ait!=active.end();ait++){
			if(ait->seqno == seqno && ait->num==num){
                //cout<<"Found seqno = "<<ait->seqno<<", num = "<<ait->num<<", frame_number = "<<ait->frame_number<<", in active list for free."<<endl;
				active.erase(ait);
				break;
			}
		}
		list<lru_node>::iterator iait;
		for(iait=inactive.begin();iait!=inactive.end();iait++){
			if(iait->seqno == seqno && iait->num==num){
                //cout<<"Found seqno = "<<iait->seqno<<", num = "<<iait->num<<", frame_number = "<<iait->frame_number<<", in inactive list for free."<<endl;
				inactive.erase(iait);
				break;
			}
		}
	//}
	//cout<<"Memsize after free : "<<free_memory.size()<<endl;
}

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

void make_buddy_list(){
    map<int,vector<vector<int> > >mp;
    mp[1] = vector<vector<int> >();
    mp[2] = vector<vector<int> >();
    mp[4] = vector<vector<int> >();
    mp[8] = vector<vector<int> >();
    mp[16] = vector<vector<int> >();
    mp[32] = vector<vector<int> >();
    mp[64] = vector<vector<int> >();
    mp[128] = vector<vector<int> >();
    mp[256] = vector<vector<int> >();
    vector<int>free_temp;
    set<int>::iterator it;
    for(it=free_memory.begin();it!=free_memory.end();it++){
        free_temp.push_back(*it);
    }
    int i=0;
    while(i<free_temp.size()){
        int j=i;
        while(j+1<free_temp.size() && (free_temp[j]==(free_temp[j+1]-1)) )
            j++;
        int res = power_two(j-i+1);
        vector<int>temp;
        for(int k=i;k<i+res;k++){
            temp.push_back(free_temp[k]);
        }
        mp[temp.size()].push_back(temp);
        i=i+res;
    }

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
	int arr[4400][2];
	char arrc[4400];
	char buffer[40];
	char temp[10];
	int i =0;
	int j =0;
	int count =0;
	if( !file ) {
		puts("Error");
		return 0;
	}
	int inputLength =0;
	while (!feof(file))
	{
		fgets(buffer, 40, file); //read line
		arrc[inputLength]=buffer[0]; // get A/X/F
		//arr1[count].operation = buffer[0];
		////////GET <seqno>//////
		i =2;
		j =0;
		while(buffer[i]!='\t'){
			temp[j]=buffer[i];
			i++;
			j++;
		}
		temp[j]='\0';
		arr[inputLength][0] = atoi(temp);
		//arr1[count].seqno = atoi(temp);
		//////////////////////////

		////////GET <num>//////
		i++;
		j=0;
		while(buffer[i]!='\n'){
			temp[j]=buffer[i];
			j++;
			i++;
		}
		temp[j]='\0';
		arr[inputLength][1] = atoi(temp);
		//arr1[count].num = atoi(temp);
		inputLength++;
		//////////////////////////
	}
	fclose(file);
	init(512);
	for(i=0;i<inputLength;i++){
		//printf("%c -> %d -> %d\n",arrc[i], arr[i][0], arr[i][1]);
		//printf("%c -> %d -> %d\n",arr1[i].operation, arr1[i].seqno, arr1[i].num);
		//sprintf("-----------\n");
		if(arrc[i]=='A'){
		    //cout<<"A -> "<<"seqno : "<<arr[i][0]<<"  num : "<<arr[i][1]<<endl;
            //print_free_memory();
            allocate_pages(arr[i][0],arr[i][1]);
		}
		else if(arrc[i]=='F'){
            //cout<<"F -> "<<"seqno : "<<arr[i][0]<<"  num : "<<arr[i][1]<<endl;
			free_page(arr[i][0],arr[i][1]);
		}
		else if(arrc[i]=='X'){
		    //cout<<"X -> "<<"seqno : "<<arr[i][0]<<"  num : "<<arr[i][1]<<endl;
			access_page(arr[i][0],arr[i][1]);
		}
	}
    //view_memory();
    cout<<"===================================="<<endl;
    print_active();
    cout<<"===================================="<<endl;
    print_inactive();
    cout<<"===================================="<<endl;

    print_free_memory();
    cout<<"===================================="<<endl;
    cout<<"Status of buddy lists : "<<endl;
    make_buddy_list();
	return 0;
}
