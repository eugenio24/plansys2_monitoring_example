import re
import sys
from pathlib import Path


# ------------------------------------------------------------
# Utilities
# ------------------------------------------------------------

def strip_comments(text):
    return re.sub(r";.*", "", text)


def extract_balanced(text, start_idx):
    depth = 0
    for i in range(start_idx, len(text)):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return text[start_idx:i + 1], i + 1
    raise ValueError("Unbalanced parentheses")

def substitute_expr(expr, mapping):
    expr = expr.strip()

    # literal number
    try:
        float(expr)
        return expr
    except ValueError:
        pass

    # variable
    if expr in mapping:
        return mapping[expr]

    # remove outer parentheses
    if expr.startswith("(") and expr.endswith(")"):
        inner = expr[1:-1].strip()
        tokens = split_lisp(inner)
        substituted = [substitute_expr(t, mapping) for t in tokens]
        return "(" + " ".join(substituted) + ")"

    return expr

def split_lisp(text):
    tokens = []
    i = 0
    while i < len(text):
        if text[i].isspace():
            i += 1
            continue
        if text[i] == "(":
            expr, j = extract_balanced(text, i)
            tokens.append(expr)
            i = j
        else:
            j = i
            while j < len(text) and not text[j].isspace():
                j += 1
            tokens.append(text[i:j])
            i = j
    return tokens


# ------------------------------------------------------------
# Numeric expression evaluation
# ------------------------------------------------------------

def eval_expr(expr, numeric_state):
    expr = expr.strip()

    # numeric literal
    try:
        return float(expr)
    except ValueError:
        pass

    if expr.startswith("(") and expr.endswith(")"):
        expr = expr[1:-1].strip()

    tokens = split_lisp(expr)
    head = tokens[0]

    if head in {"+", "-", "*", "/"}:
        a = eval_expr(tokens[1], numeric_state)
        b = eval_expr(tokens[2], numeric_state)
        return {
            "+": a + b,
            "-": a - b,
            "*": a * b,
            "/": a / b,
        }[head]

    key = tuple([head] + tokens[1:])
    if key not in numeric_state:
        raise KeyError(f"Missing function value: {key}")

    return numeric_state[key]


# ------------------------------------------------------------
# Effect parsing
# ------------------------------------------------------------

def parse_effect(effect):
    effect = effect.strip()
    while effect.startswith("(") and effect.endswith(")"):
        effect = effect[1:-1].strip()

    for op in ("increase", "decrease", "assign"):
        if effect.startswith(op):
            rest = effect[len(op):].strip()
            func_part, idx = extract_balanced(rest, 0)
            expr = rest[idx:].strip()

            func_part = func_part[1:-1].strip()
            function, *args = func_part.split()

            return {
                "type": "numeric",
                "op": op,
                "function": function,
                "args": args,
                "expr": expr
            }

    if effect.startswith("not"):
        rest = effect[3:].strip()
        while rest.startswith("(") and rest.endswith(")"):
            rest = rest[1:-1].strip()
        pred, *args = rest.split()
        return {"type": "predicate", "pred": pred, "args": args, "value": False}

    pred, *args = effect.split()
    return {"type": "predicate", "pred": pred, "args": args, "value": True}


def extract_at_effects(effect_block, tag):
    results = []
    idx = 0
    while True:
        idx = effect_block.find(f"(at {tag}", idx)
        if idx == -1:
            break
        inner_start = effect_block.find("(", idx + len(f"(at {tag}"))
        expr, idx = extract_balanced(effect_block, inner_start)
        results.append(expr)
    return results


# ------------------------------------------------------------
# Domain / plan parsing
# ------------------------------------------------------------

def parse_domain(domain_text):
    domain_text = strip_comments(domain_text)
    actions = {}

    for m in re.finditer(r"\(:durative-action\s+([^\s]+)", domain_text):
        name = m.group(1)
        block, _ = extract_balanced(domain_text, m.start())

        params = [
            p for p in re.search(r":parameters\s*\((.*?)\)", block, re.DOTALL)
            .group(1).split()
            if p.startswith("?")
        ]

        start_effects, end_effects = [], []
        eff_idx = block.find(":effect")
        if eff_idx != -1:
            eff_block, _ = extract_balanced(block, block.find("(", eff_idx))
            for e in extract_at_effects(eff_block, "start"):
                start_effects.append(parse_effect(e))
            for e in extract_at_effects(eff_block, "end"):
                end_effects.append(parse_effect(e))

        actions[name] = {
            "parameters": params,
            "start_effects": start_effects,
            "end_effects": end_effects
        }

    return actions


def parse_plan(plan_text):
    plan = []
    regex = re.compile(r"^\s*[\d.]+:\s*\(([^ ]+)\s*(.*?)\)")
    for line in plan_text.splitlines():
        m = regex.match(line)
        if m:
            plan.append({"action": m.group(1), "args": m.group(2).split()})
    return plan


# ------------------------------------------------------------
# State parsing (problem file / commands file)
# ------------------------------------------------------------

def parse_state(state_text):
    numeric_state = {}
    for line in state_text.splitlines():
        line = line.strip()
        if line.startswith("set function"):
            expr, val = line[len("set function"):].rsplit(")", 1)
            expr = expr.strip()[1:]
            val = float(val.strip())
            parts = expr.split()
            numeric_state[tuple(parts)] = val
    return numeric_state


# ------------------------------------------------------------
# Execution
# ------------------------------------------------------------

def substitute(effect, mapping):
    if effect["type"] == "predicate":
        return {
            **effect,
            "args": [mapping.get(a, a) for a in effect["args"]]
        }

    return {
        **effect,
        "args": [mapping.get(a, a) for a in effect["args"]],
        "expr": substitute_expr(effect["expr"], mapping)
    }

def execute(domain, plan, numeric_state):
    predicate_out = []
    function_out = []

    for step in plan:
        action = domain[step["action"]]
        mapping = dict(zip(action["parameters"], step["args"]))

        for eff in action["start_effects"] + action["end_effects"]:
            eff = substitute(eff, mapping)

            if eff["type"] == "predicate":
                predicate_out.append(eff)
            else:
                delta = eval_expr(eff["expr"], numeric_state)
                key = tuple([eff["function"]] + eff["args"])

                if eff["op"] == "assign":
                    numeric_state[key] = delta
                elif eff["op"] == "increase":
                    numeric_state[key] += delta
                elif eff["op"] == "decrease":
                    numeric_state[key] -= delta

                function_out.append((key, numeric_state[key]))

    return predicate_out, function_out


# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main(domain_p, plan_p, state_p, pred_out, func_out):
    domain = parse_domain(Path(domain_p).read_text())
    plan = parse_plan(Path(plan_p).read_text())
    numeric_state = parse_state(Path(state_p).read_text())

    preds, funcs = execute(domain, plan, numeric_state)

    with open(pred_out, "w") as f:
        for p in preds:
            f.write(f'{p["pred"]} {" ".join(p["args"])} '
                    f'{"true" if p["value"] else "false"}\n')

    with open(func_out, "w") as f:
        for (fn, *args), val in funcs:
            f.write(f"{fn} {' '.join(args)} {val}\n")


if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: python generate_sequence.py domain.pddl plan.txt commands predicates.txt functions.txt")
        sys.exit(1)

    main(*sys.argv[1:])
