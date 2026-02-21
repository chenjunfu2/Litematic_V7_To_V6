#pragma once

#include <type_traits>

//类-拷贝构造
#define CLASS_COPY_CONSTRUCTOR(class_name, keyword)\
class_name(const class_name &_Copy) = keyword;

//类-拷贝赋值
#define CLASS_COPY_OPERATOR(class_name, keyword)\
class_name &operator=(const class_name &_Copy) = keyword;

//类-移动-构造
#define CLASS_MOVE_CONSTRUCTOR(class_name, keyword)\
class_name(class_name &&_Move) noexcept = keyword;

//类-移动赋值
#define CLASS_MOVE_OPERATOR(class_name, keyword)\
class_name &operator=(class_name &&_Move) noexcept = keyword;

//类-无参构造
#define CLASS_CONSTRUCTOR(class_name, keyword)\
class_name(void) = keyword;

//类-无参析构
#define CLASS_DESTRUCTOR(class_name, keyword)\
~class_name(void) = keyword;


//默认-拷贝相关
#define DEFAULT_COPY(class_name)\
CLASS_COPY_CONSTRUCTOR(class_name, default)\
CLASS_COPY_OPERATOR(class_name, default)

//默认-移动相关
#define DEFAULT_MOVE(class_name)\
CLASS_MOVE_CONSTRUCTOR(class_name, default)\
CLASS_MOVE_OPERATOR(class_name, default)

//默认-无参构造
#define DEFAULT_DSTC(class_name)\
CLASS_DESTRUCTOR(class_name, default)

//默认-无参相关
#define DEFAULT_CSTC(class_name)\
CLASS_CONSTRUCTOR(class_name, default)


//删除-拷贝相关
#define DELETE_COPY(class_name)\
CLASS_COPY_CONSTRUCTOR(class_name, delete)\
CLASS_COPY_OPERATOR(class_name, delete)

//删除-移动相关
#define DELETE_MOVE(class_name)\
CLASS_MOVE_CONSTRUCTOR(class_name, delete)\
CLASS_MOVE_OPERATOR(class_name, delete)

//删除-无参构造
#define DELETE_DSTC(class_name)\
CLASS_DESTRUCTOR(class_name, delete)

//删除-无参析构
#define DELETE_CSTC(class_name)\
CLASS_CONSTRUCTOR(class_name, delete)


//虚函数-拷贝相关
#define VIRTUAL_COPY(class_name)\
virtual CLASS_COPY_OPERATOR(class_name, 0)

//虚函数-移动相关
#define VIRTUAL_MOVE(class_name)\
virtual CLASS_MOVE_OPERATOR(class_name, 0)

//虚函数-无参构造
#define VIRTUAL_DSTC(class_name)\
virtual CLASS_DESTRUCTOR(class_name, 0)


//默认-拷贝与移动相关
#define DEFAULT_CPMV(class_name)\
DEFAULT_COPY(class_name)\
DEFAULT_MOVE(class_name)

//删除-拷贝与移动相关
#define DELETE_CPMV(class_name)\
DELETE_COPY(class_name)\
DELETE_MOVE(class_name)

//虚函数-拷贝与移动相关
#define VIRTUAL_CPMV(class_name)\
VIRTUAL_COPY(class_name)\
VIRTUAL_MOVE(class_name)


//获取-引用
#define GETTER_REFE(func_name, member_name)\
typename std::remove_reference_t<decltype(member_name)> &Get##func_name(void) noexcept\
{\
	return member_name;\
}

//获取-常量引用
#define GETTER_CREF(func_name, member_name)\
const typename std::remove_reference_t<decltype(member_name)> &Get##func_name(void) const noexcept\
{\
	return member_name;\
}

//获取-拷贝
#define GETTER_COPY(func_name, member_name)\
typename std::remove_reference_t<decltype(member_name)> Get##func_name(void) const\
{\
	return member_name;\
}

//设置-拷贝
#define SETTER_COPY(func_name, member_name)\
void Set##func_name(const typename std::remove_reference_t<decltype(member_name)> &_##member_name)\
{\
	member_name = _##member_name;\
}

//设置-移动
#define SETTER_MOVE(func_name, member_name)\
void Set##func_name(typename std::remove_reference_t<decltype(member_name)> &&_##member_name) noexcept\
{\
	member_name = std::move(_##member_name);\
}

//获取-引用与常量引用
#define GETTER_RFCR(func_name, member_name)\
GETTER_REFE(func_name, member_name)\
GETTER_CREF(func_name, member_name)

//设置-拷贝与移动
#define SETTER_CPMV(func_name, member_name)\
SETTER_COPY(func_name, member_name)\
SETTER_MOVE(func_name, member_name)

