#include<sstream>
#include<iostream>
#include<string.h>
#include<string>
#include<fstream>
#include<string>
#include<cstdlib>
using namespace std;

ifstream fin;
ofstream fout;
int SI,PI,TI;
bool occupied_pages[30];

class memory {
	private:
		char mem[300][4];
		char ch;
		int page_table_ptr;
		
	public:
		void reset() {
			// reset the memory by replacing every symbol in 2D array by $
			memset(mem, '$', sizeof(char) * 300 * 4);
			memset(occupied_pages, false, sizeof(bool) * 30);
			
			page_table_ptr = rand() % 30;
			
			occupied_pages[page_table_ptr] = true;
			
			page_table_ptr *= 10;
			
			SI = 0;
			PI = 0;
			TI = 0;
		}

		string get_mem(int pos) {
			// get word form memory
			// get the memory contents of given position
			string temp = "";
			
			for(int i=0; i<4; i++)
				temp += mem[pos][i];
			
			return temp;
		}

		// store word in mem
		void set_mem(string s, int pos) {
			//set the memory for the recieved value at postion 
			for(int i=0; i<4; i++)
				mem[pos][i] = s[i];
		}

		int get_page_table_ptr() {
			return page_table_ptr;
		}

		int allocate_page() {
			int page_no = rand() % 30;
			
			while(occupied_pages[page_no] == true)
				page_no = rand() % 30;
			
			occupied_pages[page_no] = true;
			return page_no;
		}
		
		void set_page_table(int row_num, int page_no) {
			ostringstream temp;
			temp << page_no;
			
			string table_entry;
			if(page_no < 10)
				table_entry = "$10" + temp.str();
			else	
				table_entry = "$1" + temp.str();
			
			set_mem(table_entry, page_table_ptr + row_num);
		}

		void store_card(string s, int mem_cnt) {
			// extract the words and call the setmem function
			string word = "";
			
			int page_no = allocate_page();
			
			set_page_table(mem_cnt, page_no);
			
			page_no *= 10;
			
			for(int i = 0; i < s.length(); i += 4) {
				for(int j=0; j<4; j++) {
					word += s[i + j];
				}	

				set_mem(word, page_no);
				
				page_no += 1;
				
				word = "";
			}
		}

		void print_mem() {
			int flag=0;
			
			for(int i=0; i<300; i++) {
				for(int j=0; j<4; j++) {
					fout << mem[i][j] << " ";
				}	
				fout << endl;
			}
		}
} m_obj;

class cpu {
	private:
		int fetched_IC, terminate_code, TLL, TTL, LLC, TLC;
		bool terminate, fetched_C;
		string fetched_IR, operand, opreator, fetched_R, compare_string, p_id;
		char IR[4], R[4], IC[2];
		bool C, run_mos;

	public:
		// set and reset function of all the register
		int s_to_i(string operand) {
			// return the integer no for the given string
			if(operand[0] >= '0' && operand[0] <= '9' && operand[1] >= '0' && operand[1] <= '9')
				return ((int)operand[0] - 48) * 10 + ((int)operand[1] - 48);
			
			return -1;
		}

		void set_limits(string limits) {
			p_id = limits.substr(0,4);
			
			TTL = s_to_i(limits.substr(4,2)) * 100 + s_to_i(limits.substr(6, 2));
			
			TLL = s_to_i(limits.substr(8,2)) * 100 + s_to_i(limits.substr(10, 2));
			
			LLC = 0;
			TLC = 0;
			terminate_code = 0;
		}
		
		void set_IC() {
			IC[0] = '0';
			IC[1] = '0';
		}
		
		void set_IC(int pos) {
			IC[1] = ((char)pos % 10) + 48;
			pos = pos / 10;
			IC[0] = ((char)pos % 10) + 48;
		}

		int get_IC() {
			int ic_val = ((int)IC[0] - 48) * 10 + ((int)IC[1] - 48);
			return ic_val;	
		}	

		void inc_IC() {
			int ic_val = get_IC();
			
			ic_val += 1;
			
			set_IC(ic_val);
		}

		void set_IR(int IC) {
			string returned_value = "";
			returned_value = m_obj.get_mem(IC);
			
			for(int i=0; i<4; i++)
				IR[i] = returned_value[i];
		}

		string get_IR() {
			string ret_IR = "";
			
			for(int i=0;i<4;i++)
				ret_IR+=IR[i];
			
			return ret_IR;
		}

		void set_R(int pos) {
			string returned_value = "";
			
			returned_value = m_obj.get_mem(pos);
			
			for(int i=0; i<4; i++)
				R[i] = returned_value[i];
		}
		
		string get_R() {
			string ret_R="";
			
			for(int i=0;i<4;i++) {
				ret_R += R[i];
			}
			
			return ret_R;
		}

		void set_C(bool value) {
			C = value;
		}

		bool get_C() {
			return C;
		}

		int address_tranlation(int virtual_add) {
			int page = m_obj.get_page_table_ptr() + (virtual_add/10);
			string value_page = m_obj.get_mem(page);
			
			if(value_page[1] == '$') {
				PI = 3;
				return -1;
			}

			value_page = value_page.substr(2,2);
			return (s_to_i(value_page) * 10 + (virtual_add % 10));
		}
		
		void startexe() {
			// fetch decode execute cycle given below
			set_IC(0);
			
			terminate = false;
			
			while(!terminate) {
				// fetch ic
				// fetch ir form the location specified by ic
				// get the value of operator and operand
				
				run_mos = false;
				
				fetched_IC = address_tranlation(get_IC()); 	
				
				inc_IC();
				
				set_IR(fetched_IC);
				
				fetched_IR = get_IR();
				
				if((fetched_IR.compare("H")) == 3)
					fetched_IR = "Hrrr";
				
				opreator = fetched_IR.substr(0,2);
				operand = fetched_IR.substr(2,2);
				
				int pos = address_tranlation(s_to_i(operand));
				
				// TLC++;
				
				if(TLC > TTL) {
					TI = 2;
					
					run_mos = true;
					
					terminate = true;
					
					terminate_code = 3;
				}
				
				if(operand=="rr" || (s_to_i(operand) != -1)) {
					if(!(opreator.compare("LR"))) {
						// set the contents of the register R from the given location of memory specified in the operand
						// cout<<"LR";
						TLC += 1;

						if(pos == -1) {
							run_mos = true;
						}
						else
							set_R(pos);
					} else if (!(opreator.compare("SR"))) {
						// get the contents of register R
						// store them at the location of memory specified in the operand
						TLC += 2;
						
						fetched_R = get_R();
						
						if(pos == -1) {
							int page_no = m_obj.allocate_page();
							m_obj.set_page_table((s_to_i(operand))/10, page_no);
							pos = (page_no*10) + (s_to_i(operand)%10);
						}
						
						m_obj.set_mem(fetched_R, pos);
					} else if (!(opreator.compare("CR"))) {
						// get the contents of register r
						// compare with given memory location
						// if the values of above two matches then set toggle register to true
						// else set the toggle register to false
						TLC += 1;

						fetched_R = get_R();
						if(pos == -1) {
							run_mos = true;
							PI = 3;
						} else {
							compare_string = m_obj.get_mem(pos);
							
							if(fetched_R.compare(compare_string) == 0)
								set_C(true);
							else
								set_C(false);
						}
					} else if (!(opreator.compare("BT"))) {
						// if the value of toggle register is true 
						// we change the value of ic
						TLC += 1;

						fetched_C = get_C();
						
						if(fetched_C) {
							//int get_physical_add = address_tranlation();
							set_IC(s_to_i(operand));
						} else {
							set_IC(s_to_i(operand) + 2);
						}
					} else if (!(opreator.compare("GD"))){
						TLC += 2;
						
						SI = 1;
						
						run_mos = true;
					} else if (!(opreator.compare("PD"))){
						TLC += 1;

						if(pos==-1)
							PI = 3;
						else
							SI = 2;
						
						run_mos = true;
					} else if (!(opreator.compare("Hr"))){
						TLC += 1;

						fetched_IR = "H";
						SI = 3;
						run_mos = true;
					} else if(!(opreator.compare("$$"))){
						terminate = true;
						terminate_code = 1;
						run_mos = true;
					} else {
						PI = 1;
						run_mos = true;
					}
				} else {
					PI = 2;
					run_mos = true;
				}
				
				if(run_mos)
					MOS();
			}
		}
		
		void MOS() {
			if(TI==0 && SI==1) {
				// get the instrution of the file in terms of 4 words 
				// store it one by one into memory
				// at start store the program card from memory location with unit place 0

				string s;
				int pos = address_tranlation(s_to_i(operand));
				if(pos==-1) {
					int page_no = m_obj.allocate_page();
					m_obj.set_page_table((s_to_i(operand))/10,page_no);
					pos = (page_no*10) + (s_to_i(operand)%10);
				}
				
				pos = (pos/10) * 10;
				
				getline(fin,s);
				
				if(s.find("$END") != -1) {
					terminate = true;
					terminate_code = 1;
				} else {
					if(!s.empty() && s[s.size()-1] == '\r')
						s.erase(s.size()-1);
					
					int len = s.length(), start=0, i;
					
					string s1;
					
					for(i=pos; start<len; i++) {
						if((len-start) < 4)
							s1=s.substr(start, (len-start));
						else	
							s1=s.substr(start, 4);
						
						start += 4;
						m_obj.set_mem(s1, i);
					}
				}

				SI=0;
			} else if(TI == 2 && SI == 1) {
				terminate = true;
				terminate_code = 3;
			} else if((TI == 0 || TI == 2) && SI == 2) {
				LLC += 1;
				if(LLC > TLL) {
					terminate = true;
					terminate_code = 2;
				} else {
					// put the data from memory into the file specified

					int pos = address_tranlation(s_to_i(operand)), flag = 0;
					pos = (pos/10) * 10;
					string ans = "", temp="";
					for(int i=pos; i<pos+10; i++) {
						temp = m_obj.get_mem(i);
						
						for(int j=0;j<4;j++) {
							if(temp[j]=='\0' || temp[j]=='$') {
								break;
								flag=1;
							}
							ans+=temp[j];
						}
						if(flag)
							break;
					}
					
					fout << ans << endl;
					
					if(TI==2) {
						terminate = true;
						terminate_code = 3;
					}
				}

				SI=0;
			} else if((TI==0 || TI==2) && SI==3) {
				//make terminate true to end the program
				//as we encountered the halt instruction 
				terminate = true;
				terminate_code = 0;
			} else if(TI==0 && PI==1) {
				terminate_code = 4;
				terminate = true;
			} else if(TI==0 && PI==2) {
				terminate_code = 5;
				terminate = true;
			} else if(TI==0 && PI==3) {
				terminate_code = 6;
				terminate = true;
			} else if(TI==2 && PI==1) {
				terminate_code = 7;
				terminate = true;
			} else if(TI==2 && PI==2) {
				terminate_code = 8;
				terminate = true;
			} else if(TI==2 && PI==3) {
				terminate_code = 3;
				terminate = true;
			}
			
			if(terminate) {
				fout << p_id << " ";
				
				switch(terminate_code) {
					case 0: fout << "NO ERROR\n";
					       	break;
					case 1: fout << "OUT OF DATA ERROR\n";
					        break;
					case 2: fout << "LINE LIMIT EXCEEDED ERROR\n";
					        break;
					case 3: fout << "TIME LIMIT EXCEEDED ERROR\n";
					        break;
					case 4: fout << "OPERATION CODE ERROR\n";
					        break;
					case 5: fout << "OPERAND ERROR\n";
					        break;
					case 6: fout << "INVALID PAGE FAULT ERROR\n";
					        break;
					case 7: fout << "TIME LIMIT EXCEEDED with OPERATION CODE ERROR\n";
					        break;
					case 8: fout << "TIME LIMIT EXCEEDED with OPERAND ERROR\n";
					        break;
				}
				
				fout << get_IC() << " " << fetched_IR << " " << fetched_C << " " << TLC << " " << LLC << endl; 
				fout << endl << endl;
			}
		}
} exe;

int main() {

	// open two files one input and one output
	// create memory and cpu object

	fin.open("input.txt");
	fout.open("output.txt");
	
	string inputJob;
	
	int mem_cnt = 0;
	
	while(!(fin.eof())) {
		// get the line one by one and check whether it contains as follows

		getline(fin, inputJob);
		if(inputJob.empty())
			break;

		if(inputJob.find("$AMJ") != -1) {
			// reset the memory
			
			m_obj.reset();
			mem_cnt = 0;
			
			exe.set_limits(inputJob.substr(4,12));
			
			continue;
		} else if(inputJob.find("$DTA") != -1) {	
			// m_obj.print_mem();
			exe.startexe();
		} else if(inputJob.find("$END") != -1) {
			// proceed to the next job
			mem_cnt = 0;
			m_obj.reset();

			continue;
		} else {
			// load the program card as soon as it comes
			m_obj.store_card(inputJob, mem_cnt++);
		}
	}
	
	fin.close();
	fout.close();
	
	return 0;
}
