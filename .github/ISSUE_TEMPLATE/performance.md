---
name: Performance
about: Refactor, speed optimization, or architectural changes.
title: ''
labels: type:performance
assignees: ''

---

## 🛠 Description
*What technical change are you planning? (e.g., "Switching from string concatenation to buffer writes").*

## 📈 Expected Impact
*Why is this necessary? Will it reduce CPU usage, memory footprint, or increase throughput?*

## 🧪 Benchmark Plan (Required for Performance)
*How will we verify the improvement? (e.g., using `hyperfine` or `time`)*

- [ ] Command: `cat large_file.json | jpipe run ...`
- [ ] Target: At least 2% reduction in processing time.

## 🏁 Checklist

- [ ] No breaking changes to existing CLI flags.
- [ ] Memory leak check performed.
- [ ] Platform tested on (macOS/Linux).
