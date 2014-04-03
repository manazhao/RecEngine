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
3: double ent_val
}


service HandleData{
string add_entity(1:string entityJson),
string add_interaction(1:string interactionJson),
string get_recommend_list(1:string userId),
map<byte,list<i64> > get_entity_ids(),
map<byte,list<Interact> > get_entity_interacts(1:i64 entId),
}

