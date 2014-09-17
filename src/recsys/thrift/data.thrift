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
2: double ent_val
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

struct Recommendation{
1:string id,
2:byte type,
3:double score,
}

enum StatusCode{
SC_FAIL,
SC_SUCCESS
}

struct Response{
1:StatusCode status,
2:string message
}

service DataHost{
Response index_interaction(1:string fromId, 2:byte fromType, 3:string toId, 4:byte toType, 5:byte type, 6:double val),
map<byte,list<Interact> > query_entity_interacts(1:i64 id), /// used by recommendation engine
string query_entity_name(1:i64 id), /// used by web server: get the original entity id
list<string> query_entity_names(1:list<i64> idList), /// the same purpose as above function, except in batch mode
i64 query_entity_id(1:string name, 2:byte type),
Dataset get_dataset(1:DSType dsType),
Dataset get_cv_train(1:byte foldIdx),
Dataset get_cv_test(1:byte foldIdx)
}

service RecEngine{
list<Recommendation> get_recommendation(1:string userId),
}

