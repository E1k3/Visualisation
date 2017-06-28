#include "globject.h"

GLObject::GLObject(unsigned id, std::function<void(unsigned)> deleter) :
	_id{id},
	_deleter{deleter}
{

}

GLObject::GLObject(GLObject&& other) :
	_id{other.release()},
	_deleter{other._deleter}
{

}

GLObject& GLObject::operator=(GLObject&& other)
{
	_deleter(_id);
	_id = other.release();
	_deleter = other._deleter;
	return *this;
}

GLObject::~GLObject()
{
	if(_id)
		_deleter(_id);
}

unsigned GLObject::release()
{
	auto id = _id;
	_id = 0;
	return id;
}

void GLObject::reset(unsigned id)
{
	_deleter(_id);
	_id = id;
}

void GLObject::reset(unsigned id, std::function<void(unsigned)> deleter)
{
	_deleter(_id);
	_id = id;
	_deleter = deleter;
}

unsigned GLObject::get() const
{
	return _id;
}

