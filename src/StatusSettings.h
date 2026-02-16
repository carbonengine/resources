// Copyright © 2025 CCP ehf.

#pragma once
#ifndef StatusSettings_H
#define StatusSettings_H

#include <memory>
#include <string>
#include <sstream>
#include "Exports.h"
#include "ResourceGroup.h"


namespace CarbonResources
{

struct StatusReturn
{
	float progress = 0;
	float scale = 1;
};

struct StatusUpdate
{
	StatusProgressType statusProgressType = StatusProgressType::UNBOUNDED;
	float progress = 0;
	float percentageSizeOfJob = 0;
	std::string info;
};

class StatusSettings
{
public:
	StatusSettings( const StatusSettings& ) = delete;

	StatusSettings& operator=( const StatusSettings& ) = delete;

	StatusSettings();

	~StatusSettings();

	bool RequiresStatusUpdates() const;

	void SetCallbackSettings( const CallbackSettings& callbackSettings );

	void Update( StatusProgressType statusProgressType, float progress, float percentageSizeOfJob, const std::string& info, StatusSettings* nestedStatusSettingsOut = nullptr );

private:
	StatusReturn CalculateOverallProgress() const;

private:
	StatusUpdate m_lastUpdate;

	StatusSettings* m_parent;

	CallbackSettings m_callbackSettings;

	int m_nestingLevel;
};

}

#endif // StatusSettings_H