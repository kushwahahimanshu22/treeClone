#include<iostream>
#include<filesystem>
#include<string>
#include<vector>

using namespace std;

namespace fs = filesystem;

class Directory{
    public:
    fs::path absPath;
    string name;
    int size;
    string perms; 
    fs::file_status s;
    vector<Directory*> childs;
    Directory(fs::path path){
        this->absPath = fs::absolute(path);
        this->name = path.filename();
        this->s = fs::status(path);
        if(!fs::is_directory(path))
            this->size = fs::file_size(path);

        this->perms = "";
        fs::perms pT = this->s.permissions();
        auto adder = [=](char op, fs::perms p){
            this->perms += (fs::perms::none == (p & pT) ? '-' : op);
        };
        adder('r', fs::perms::owner_read);
        adder('w', fs::perms::owner_write);
        adder('x', fs::perms::owner_exec);
        adder('r', fs::perms::group_read);
        adder('w', fs::perms::group_write);
        adder('x', fs::perms::group_exec);
        adder('r', fs::perms::others_read);
        adder('w', fs::perms::others_write);
        adder('x', fs::perms::others_exec);

        if(fs::is_directory(path)){
            for(auto const& dir_entry : fs::directory_iterator(path)){
                this->childs.push_back(new Directory(dir_entry.path()));
            }
        }
    }
    void printTree(string prefix, vector<bool> &flags){
        if(flags[1] && !fs::is_directory(this->absPath)) return;
        cout << prefix+"├─ "+(flags[2]?this->absPath.string():this->name) << endl;
        if(fs::is_directory(fs::path(this->absPath))){
            for(auto &x : this->childs){
                if(!flags[0] && (x->name[0] == '.' && (x->name.size() > 1))) continue;
                
                x->printTree(prefix+"   ", flags);
            }
        }
    }
};

int main(int argc, char **argv){
    vector<bool> flags = {0,0,0,0,0};
    fs::path path = "";
    for(int i=1;i<argc;i++){
        if(argv[i][0] == '-'){
            string argTemp = argv[i];
            for(int j=1;j<argTemp.size();j++){
                if(argTemp[j] == 'a'){
                    flags[1] = 1;
                }
                else if(argTemp[j] == 'd'){
                    flags[0] = 1;
                }
                else if(argTemp[j] == 'h'){
                    cout << "Usage ./tree PATH [-FLAGS]\nFlags:\n1)d - Display only directories\n2)a - Display hidden files also\n3)f - Display Full path\n";
                    return 0;
                }
                else if(argTemp[j] == 'f'){
                    flags[2] = 1;
                }
                else{
                    cout << "INVALID FLAGS" << endl;
                    exit(1);
                }
            }
        }
        else{
            if(path == "")
                path = argv[i];
        }
    }
    Directory d(path);
    if(d.name == "") d.name = ".";
    d.printTree(" ", flags);
    return 0;
}
