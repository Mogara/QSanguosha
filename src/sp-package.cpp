#include "sp-package.h"
#include "general.h"
#include "skill.h"

SPPackage::SPPackage()
    :Package("sp")
{
    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Skill("yicong", Skill::Compulsory));
}

ADD_PACKAGE(SP);
