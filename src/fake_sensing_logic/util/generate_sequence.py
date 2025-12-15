import re
import sys
from pathlib import Path

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

def parse_effect(effect):
    """
    Correctly handles:
      (robot_at ?r ?r1)
      (not (robot_at ?r ?r1))
      (not(robot_at ?r ?r1))
    """
    effect = effect.strip()

    # Remove outer parentheses
    while effect.startswith("(") and effect.endswith(")"):
        effect = effect[1:-1].strip()

    # Detect true negation: not( ... ) OR not ( ... )
    if effect.startswith("not"):
        rest = effect[3:].strip()

        if rest.startswith("("):
            # Proper negation
            while rest.startswith("(") and rest.endswith(")"):
                rest = rest[1:-1].strip()

            pred, *args = rest.split()
            return pred, args, False

    # Otherwise: positive predicate
    pred, *args = effect.split()
    return pred, args, True

def extract_at_effects(effect_block, tag):
    """
    Extracts balanced expressions from:
      (at start (...))
      (at end (...))
    """
    results = []
    idx = 0
    while True:
        idx = effect_block.find(f"(at {tag}", idx)
        if idx == -1:
            break

        # find first '(' after tag
        inner_start = effect_block.find("(", idx + len(f"(at {tag}"))
        expr, next_idx = extract_balanced(effect_block, inner_start)
        results.append(expr)
        idx = next_idx

    return results

def parse_domain(domain_text):
    domain_text = strip_comments(domain_text)
    actions = {}

    for match in re.finditer(r"\(:durative-action\s+([^\s]+)", domain_text):
        name = match.group(1)
        start = match.start()
        block, _ = extract_balanced(domain_text, start)

        # Parameters
        params_match = re.search(r":parameters\s*\((.*?)\)", block, re.DOTALL)
        params = [p for p in params_match.group(1).split() if p.startswith("?")]

        start_effects = []
        end_effects = []

        eff_idx = block.find(":effect")
        if eff_idx != -1:
            eff_start = block.find("(", eff_idx)
            eff_block, _ = extract_balanced(block, eff_start)

            start_exprs = extract_at_effects(eff_block, "start")
            end_exprs   = extract_at_effects(eff_block, "end")

            for e in start_exprs:
                start_effects.append(parse_effect(e))

            for e in end_exprs:
                end_effects.append(parse_effect(e))

        actions[name] = {
            "parameters": params,
            "start_effects": start_effects,
            "end_effects": end_effects
        }

    return actions

def parse_plan(plan_text):
    """
    Parses a plan file that may contain planner logs before the actual plan. (/tmp/plan saved by default by plansys2)
    Extracts only valid timed action lines.

    Expected action format:
      time: (action arg1 arg2 ...) [duration]
    """
    plan = []

    action_line = re.compile(
        r"^\s*([\d.]+):\s*\(([^ ]+)\s*(.*?)\)\s*\[\s*[\d.]+\s*\]\s*$"
    )

    for line in plan_text.splitlines():
        match = action_line.match(line)
        if not match:
            continue

        _, action, args = match.groups()
        plan.append({
            "action": action,
            "args": args.split() if args else []
        })

    return plan

def substitute(effect, mapping):
    pred, args, val = effect
    return pred, [mapping.get(a, a) for a in args], val

def execute(domain, plan):
    output = []

    for step in plan:
        action = domain[step["action"]]
        mapping = dict(zip(action["parameters"], step["args"]))

        for eff in action["start_effects"]:
            output.append(substitute(eff, mapping))

        for eff in action["end_effects"]:
            output.append(substitute(eff, mapping))

    return output

def main(domain_path, plan_path, output_path):
    domain_text = Path(domain_path).read_text()
    plan_text = Path(plan_path).read_text()

    domain = parse_domain(domain_text)
    plan = parse_plan(plan_text)
    effects = execute(domain, plan)

    with open(output_path, "w") as f:
        for pred, args, val in effects:
            f.write(f"{pred} {' '.join(args)} {'true' if val else 'false'}\n")

    print(f"Written {len(effects)} effects to {output_path}")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python generate_sequence.py domain.pddl plan.txt output.txt")
        sys.exit(1)

    main(sys.argv[1], sys.argv[2], sys.argv[3])
