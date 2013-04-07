#ifndef __BTA_FIGHTING__
#define __BTA_FIGHTING__

#include "Model.h"

struct FighterPtr
{
private:
	bool isPlanet_;
	union
	{
		Fleet* fleet_;
		Planet* planet_;
	};
public:

	explicit FighterPtr(Fleet* fleet): isPlanet_(false), fleet_(fleet) {};
	explicit FighterPtr(Planet* planet): isPlanet_(true), planet_(planet) {};
	bool isPlanet() const {return isPlanet_;}
	Fleet* getFleet() const
	{
		if(isPlanet_)
			BOOST_THROW_EXCEPTION(std::logic_error("It is a planet!!"));
		else
			return fleet_;
	}
	Planet* getPlanet() const
	{
		if(false == isPlanet_)
			BOOST_THROW_EXCEPTION(std::logic_error("It is a fleet!!"));
		else
			return planet_;
	}

	bool operator == (FighterPtr const& other) const
	{
		return isPlanet_ == other.isPlanet_ &&
		       (void*)fleet_ == (void*)other.fleet_;
	}
};

//! Planet peut etre NULL
void fight(std::vector<Fleet*> const& fleetList,
           Planet* planet,
           FightReport& reportList);


#endif //__BTA_FIGHTING__