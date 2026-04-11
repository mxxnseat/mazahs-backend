## Backend Overview

A distributed C++ backend for real-time audio fingerprinting and recognition, inspired by Shazam.

The system is composed of three services:

- **Searcher (Offline Processing)**
  Processes audio files through a DSP pipeline (resampling, STFT, constellation peak extraction, hash generation).
  Fingerprints are generated asynchronously using Redis-based job queues (cppq) and persisted in PostgreSQL for efficient lookup.

- **API Service**
  Exposes REST endpoints for song management, indexing workflows, and metadata retrieval.
  Acts as the orchestration layer between clients and the processing pipeline.

- **WebSocket Service (Real-time Recognition)**
  Streams microphone input from clients, processes audio in chunks, extracts hashes on-the-fly, and performs database matching using time-offset voting to identify songs.

---

## ⚙️ Environment Variables

Each service requires the following environment variables:

```env
DATABASE_URL=postgres://postgres:postgres@localhost:5432/shazam
REDIS_HOST=localhost
REDIS_PORT=6379
```

---

## ▶️ Running Services

Make sure PostgreSQL and Redis are running locally, then start each service independently:

```bash
# Example
xmake run searcher
xmake run api
xmake run websocket
```

## The project services

- [Infrastructure services](https://github.com/mxxnseat/mazahs-infrastructure)

- [Frontend app](https://github.com/mxxnseat/mazahs-frontend)

- [Song classifier](https://github.com/mxxnseat/mazahs-song-classifier)
