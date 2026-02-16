// Copyright © 2025 CCP ehf.

#include "StatusSettings.h"

namespace CarbonResources
{
	
StatusSettings::StatusSettings() :
	m_parent( nullptr ),
	m_nestingLevel( 0 )
	
{
}

StatusSettings ::~StatusSettings()
{
	Update( StatusProgressType::END, 100, 0, "Process complete." );
}

void StatusSettings::SetCallbackSettings( const CallbackSettings& callbackSettings )
{
	m_callbackSettings = callbackSettings;
}

void StatusSettings::Update( StatusProgressType statusProgressType, float progress, float percentageSizeOfJob, const std::string& info, StatusSettings* nestedStatusSettingsOut /*= nullptr*/ )
{
	if( !m_callbackSettings.statusCallback )
	{
		return;
	}
	else if(( m_nestingLevel >= m_callbackSettings.verbosityLevel ) && ( m_callbackSettings.verbosityLevel != -1 ) )
    {
		return;
    }
	else
    {
		// Update last update
		m_lastUpdate.statusProgressType = statusProgressType;
		m_lastUpdate.progress = progress;
		m_lastUpdate.percentageSizeOfJob = percentageSizeOfJob;
		m_lastUpdate.info = info;

		StatusReturn scaledProgressAndScale = CalculateOverallProgress();

		m_callbackSettings.statusCallback( statusProgressType, progress, scaledProgressAndScale.progress, percentageSizeOfJob, m_nestingLevel, info );

		if( nestedStatusSettingsOut )
		{
			nestedStatusSettingsOut->m_parent = this;
			nestedStatusSettingsOut->m_callbackSettings = m_callbackSettings;
			nestedStatusSettingsOut->m_nestingLevel = m_nestingLevel + 1;
			nestedStatusSettingsOut->Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );
		}
	}
}

bool StatusSettings::RequiresStatusUpdates() const
{
    // Must have a valid callback
    // Nesting level must be less than or equal to verbosity level or Verbosity level of -1
	return ( m_callbackSettings.statusCallback != nullptr ) && ( ( m_nestingLevel <= m_callbackSettings.verbosityLevel ) || ( m_callbackSettings.verbosityLevel == -1 ) );
}

StatusReturn StatusSettings::CalculateOverallProgress() const
{
	StatusReturn progressAndScale = { 0, 0 };

	if( m_parent )
	{
		StatusReturn parentProgressAndScale = m_parent->CalculateOverallProgress();

		// Scale current progress by the parents step
		float scaledProgress = m_lastUpdate.progress * parentProgressAndScale.scale;
		float scaledScale = ( m_lastUpdate.percentageSizeOfJob / 100 ) * parentProgressAndScale.scale;

		StatusReturn scaledProgressAndScale = { parentProgressAndScale.progress + scaledProgress, scaledScale };

		return scaledProgressAndScale;
	}
	else
	{
		StatusReturn progressAndScale;
		progressAndScale.progress = m_lastUpdate.progress;

		progressAndScale.scale = m_lastUpdate.percentageSizeOfJob / 100;

		return progressAndScale;
	}
}

}