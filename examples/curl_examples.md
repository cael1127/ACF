### HTTP API curl examples

Assuming `compound_ai_server` is running on `http://localhost:8080`.

- **Stateless completion**

```bash
curl -X POST http://localhost:8080/v1/complete \
  -H "Content-Type: application/json" \
  -d '{"prompt":"Say hello"}'
```

- **Agent session**

```bash
curl -X POST http://localhost:8080/v1/agent \
  -H "Content-Type: application/json" \
  -d '{"session_id":"demo","input":"Inspect src/main/cli.c"}'
```

- **List tools**

```bash
curl http://localhost:8080/v1/tools
```

