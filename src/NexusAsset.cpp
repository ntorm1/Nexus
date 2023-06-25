

#include "NexusAsset.h"

NexusAsset::NexusAsset(
    NexusEnv const* nexus_env_,
    ads::CDockWidget* DockWidget_,
    QWidget* parent) :
    QMainWindow(parent),
    nexus_env(nexus_env_),
    DockWidget(DockWidget_)
{

}