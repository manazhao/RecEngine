/**
* define two services: 
* 1) add entity which include both user and item
* 2) add user activity
*/

namespace cpp recsys.thrift
namespace php recsys.thrift
namespace perl recsys.thrift


struct Interact{
1: i64 ent_id,
2: byte ent_type,
3: binary int_val
}


service HandleData{
string import_entities(1:string entityJson),
string add_entity(1:string entityJson),
string add_activity(1:string activityJson),
string get_recommend_list(1:string userId),
i64 get_max_id(1:byte entType),
map<byte,list<Interact> > get_interacts(1:i64 entId, 2:byte entType),
}

