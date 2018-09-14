#pragma once
#include "Object.h"

class Food : public Object
{
public:
	Food();

	Food(Object *pObject);

	~Food();

	virtual void initialize();
	virtual void use();
};
