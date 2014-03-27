/**
* define two services: 
* 1) add entity which include both user and item
* 2) add user activity
*/

namespace cpp recsys
namespace php recsys
namespace perl recsys

service HandleData{
string import_entities(1:string entityJson),
string add_entity(1:string entityJson),
string add_activity(1:string activityJson)
string get_recommend_list(1:string userId)
}

