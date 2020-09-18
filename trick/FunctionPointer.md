
## 函数指针

    函数指针本身是一个指针变量，该指针变量指向一个具体的函数
    代码例子：
    int test(int a)
    {
        return a;
    }
    int main(int argc, const char * argv[])
    {
        
        int (*fp)(int a);//函数指针需要和对应函数匹配
        fp = test;
        cout<<fp(2)<<endl;
        return 0;
    }

## 函数指针数组

#### 代码例子

    void func0(int); 
    void func1(int);
    void func2(int);   
    
    int main(){
      
      //函数指针数组，数组中函数返回值必须、传递的参数等必须是一致的。
      //如果参数类型不一样该怎么办？
      void (*f[3])(int) = {func0,func1,func2};
      int choice;
      cin>>choice;
      
      while((choice>=0) && (choice<3)){
          (*f[choice])(choice);
          cin>>choice;
      }
      return 0;
    }
    void func0(int a){
      cout<<a<<"func0 is called";
    }
    void func1(int b){
      cout<<b<<"func1 is called";
    }
    void func2(int c){
      cout<<c<<"func2 is called";
    }
    