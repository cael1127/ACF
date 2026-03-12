#ifndef SAFETY_GUARDRAILS_H
#define SAFETY_GUARDRAILS_H

int guardrails_is_tool_allowed(const char *agent_role, const char *tool_name, int is_write);

#endif /* SAFETY_GUARDRAILS_H */

