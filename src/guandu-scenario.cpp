#include "scenario.h"
#include "skill.h"
#include "guandu-scenario.h"

GuanduScenario::GuanduScenario()
    :Scenario("guandu")
{
    role_map["yuanshao"] = "lord";
    role_map["shuangxiong"] = role_map["zhenji"] = "loyalist";
    role_map["caocao"] = role_map["zhangliao"] = role_map["guojia"] = "rebel";
    role_map["liubei"] = role_map["guanyu"] = "renegade";

    t["guandu"] = tr("guandu");
}

ADD_SCENARIO(Guandu);


