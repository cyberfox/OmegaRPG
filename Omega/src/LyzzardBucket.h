#pragma once
#include "Food.h"

class LyzzardBucket : public Food
{
public:
	LyzzardBucket();

	explicit LyzzardBucket(Object *pObject);

	~LyzzardBucket();

private:
	void initialize() override;
};
