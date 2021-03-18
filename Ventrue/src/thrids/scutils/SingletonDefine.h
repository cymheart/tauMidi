#ifndef SINGLETON_DIFINE_H_  
#define SINGLETON_DIFINE_H_  

namespace scutils
{

//单件类宏  
#define SINGLETON(_CLASS_)                              \
public:                                                 \
	inline static _CLASS_& GetInstance()                \
   {                                                    \
       if (nullptr == _instance)                           \
       {                                                \
	        static _CLASS_ inst;                        \
			_instance = &inst;	                        \
       }                                                \
       return *_instance;                               \
   }                                                   \
private:                                                \
	_CLASS_();                                         \
	_CLASS_(_CLASS_ const&) : _CLASS_() {}               \
	_CLASS_& operator= (_CLASS_ const&) { return *this; }  \
	~_CLASS_();                                           \
private:                                                    \
	 static _CLASS_* _instance;                           


#define BUILD_SHARE(_CLASS_)\
   _CLASS_* _CLASS_::_instance = nullptr; 

}

#endif //SINGLETON_DIFINE_H_  