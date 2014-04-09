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

enum DSType{
DS_ALL, /// all data
DS_TRAIN, /// for modeling training
DS_TEST, /// for model testing
DS_CS /// for coldstart evaluation
}

struct Dataset{
1: map<byte,set<i64> > type_ent_ids,
2: set<i64> ent_ids,
3: list<map<byte,list<Interact>>> ent_type_interacts
}

service HandleData{
string add_entity(1:string entityJson),
string add_interaction(1:string interactionJson),
string get_recommend_list(1:string userId),
Dataset get_dataset(1:DSType dsType),
}

