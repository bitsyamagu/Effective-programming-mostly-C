#include<stdio.h>

int length(char* str){
    int len = 0;
    char* p = str;
    while(*p != '\0'){
        len++;
        p++;
    }
    return len;
}

char* _buf_;
char* _cur_;
char* _end_;
char* tokenize(char* src, char delim){
    if(src != NULL){
        _buf_ = src;
        _end_ = src + length(src);
        for(_cur_ = _buf_; _cur_ < _end_ && *_cur_!='\0'; _cur_++){
            if(*_cur_ == delim){
                *_cur_ = '\0';
                _cur_++;
                return _buf_;
            }
        }
        return _buf_;
    }else {
        if(_end_ == _cur_){
            return NULL;
        }
        _buf_ = _cur_;
        for( ; _cur_ < _end_ && *_cur_!='\0'; _cur_++){
            if(*_cur_ == delim){
                *_cur_ = '\0';
                _cur_++;
                return _buf_;
            }
        }
        return _buf_;
    }
    return NULL;
}

int main(int argc, char** argv){
    char str[256];
    sprintf(str, "Effective programming mostly in C");

    char* token = tokenize(str, ' ');
    printf("%s\n", token);
    while(NULL != (token = tokenize(NULL, ' '))){
        printf("%s\n", token);
    }
    return 0;
}
