module;

module searcher.songs;

import redis;
import std;
import jobs;

Searcher::Songs::SongsQueue::SongsQueue(const RedisClient& redis) : 
    Jobs::Queue<SongsPullPayload>(redis, "songs") {}

Searcher::Songs::SongsHashesQueue::SongsHashesQueue(const RedisClient& redis) : 
    Jobs::Queue<SongsHashesExtractPayload>(redis, "songs_hashes") {}

