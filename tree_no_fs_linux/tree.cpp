#include<iostream>
#include<cstdio>
#include<memory>
#include<stdexcept>
#include<array>
#include<vector>
#include<sstream>

using namespace std;

string exec(string cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}


vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    stringstream ss(str);

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

vector<string> getSpaceSep(string str){
    vector<string> tokens;
    istringstream ss(str);
    string token;
    while(ss >> token){
        tokens.push_back(token);
    }
    return tokens;
}


class Directory{
    public:
    string absPath;
    string name;
    int size;
    string perms;
    string status;
    vector<Directory*> childs;

    Directory(string path, bool isDir, bool isLink){
        this->absPath = exec("realpath \""+path+"\"");
        vector<string> temp = split(this->absPath, '/');
        this->name = temp[temp.size()-1];

        string lsOut = exec("ls -la \""+path+"\"");
        temp.clear();
        temp = split(lsOut, '\n');
        vector<string> temp2;

        if(isDir) this->status = "dir";
        else {
            if(isLink) this->status = "link";
            else this->status = "file";
        }

        if(isDir)
            temp2 = getSpaceSep(temp[1]);
        else
            temp2 = getSpaceSep(temp[0]);
        
        this->perms = temp2[0].substr(1);
        this->size = stoi(temp2[4]);

        if(isDir){
            for(int i=3;i<temp.size();i++){
                // cout << temp[i] << endl;
                vector<string> childDetails = getSpaceSep(temp[i]);
                bool isChildDir = false;
                bool isChildLink = false;
                if(childDetails[0][0] == 'd'){
                    isChildDir = true;
                }
                else if(childDetails[0][0] == 'l'){
                    isChildLink = true;
                }
                string childName = "";
                for(int j=8;j<childDetails.size();j++){
                    childName += childDetails[j];
                    // if(isChildLink && j <= childDetails.size()-2){
                    //     cout << childDetails[j] << endl;
                    // }
                    if(j != childDetails.size()-1){
                        if(childDetails[j+1] == "->") break;
                        childName += " ";
                    }
                }
                // cout <<  path+"/"+childName << endl;
                childs.push_back(new Directory(path+"/"+childName, isChildDir, isChildLink));
            }
        }
    }
    void printTree(string prefix, vector<bool> &flags){
        try{
            if(flags[0] && this->status != "dir") return;
            cout << prefix+"├─ "+((flags[2])?this->absPath:this->name);
            if(this->status == "dir"){
                for(auto &x : this->childs){
                    if(!flags[1] && x->name[0] == '.') continue;
                    x->printTree(prefix+"   ", flags);
                }
            }
        }
        catch(exception e){
            cout << e.what() << endl;
            return;
        }
    }

};



int main(int argc, char** argv){
    if(argc < 2){
        return 1;
    }
    vector<bool> flags = {0,0,0,0,0};
    string path = "";
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
    if(path == "") {
        cout << "No Path given" << endl;
        return(1);
    }
    if(system(("test -e"+path).c_str())){
        return 1;
    }
    vector<string> temp = split(exec("ls -la "+path), '\n');
    if(temp.size() == 1){
        cout << exec("realpath \""+path+"\"") << endl;
        return 0;
    }

    Directory d(path, true, false);
    d.printTree("", flags);
    return 0;
}