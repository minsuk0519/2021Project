#pragma once
#include "Object.hpp"

class Wall : public Object
{
private:
public:
	void Initialize() override;
	void Update() override;
	void Close() override;

	virtual Object* CopyObject() const override;

	void Draw() override;
	void DrawShadow() override;
};

