## Quick start (Docker)

Requires: Docker, MinIO client (`mc`).

```bash
# Start MinIO + s3_gateway
make run-docker

# Stop everything
make docker-down
```

MinIO console is at http://localhost:9001 (user: `minioadmin`, password: `minioadmin`).

## REST API

| Method | Endpoint                   | Description                                        |
|--------|----------------------------|----------------------------------------------------|
| GET    | `/status`                  | Service health, bucket names, file count           |
| GET    | `/readines`                | Kubernetes readiness probe                         |
| GET    | `/healthcheck`             | Kubernetes liveness probe                          |
| GET    | `/list`                    | All registered files with bucket type and route    |
| GET    | `/update?filename=<name>`  | Sync and return current location of a file         |
| POST   | `/relocate?filename=<name>`| Verify file, relocate between buckets, return ok/fail |
| GET    | `/metrics`                 | Prometheus metrics for file volumes and uploads    |
| GET    | `/<bucket>/<key>`          | 302 redirect to S3 URL (public or presigned)       |
