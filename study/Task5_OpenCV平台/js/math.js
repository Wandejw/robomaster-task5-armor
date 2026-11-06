(() => {
  "use strict";

  const lines = Array.isArray(window.TASK5_MATH_LINES) ? window.TASK5_MATH_LINES : [];
  const requestedId = new URLSearchParams(window.location.search).get("task") || "math1";
  const line = lines.find((item) => item.id === requestedId) || lines[0];
  const bridgePrefix = "TASK5_OFFLINE_STATE:";

  function readBridge() {
    try {
      return window.name.startsWith(bridgePrefix)
        ? JSON.parse(window.name.slice(bridgePrefix.length))
        : {};
    } catch {
      return {};
    }
  }

  function updateBridge(patch) {
    window.name = bridgePrefix + JSON.stringify({ ...readBridge(), ...patch });
  }

  function readProgress() {
    try {
      const localProgress = JSON.parse(localStorage.getItem("task5-math-progress") || "{}");
      const progress = { ...localProgress, ...(readBridge().mathProgress || {}) };
      localStorage.setItem("task5-math-progress", JSON.stringify(progress));
      updateBridge({ mathProgress: progress });
      return progress;
    } catch {
      return {};
    }
  }

  function saveProgress(progress) {
    try {
      localStorage.setItem("task5-math-progress", JSON.stringify(progress));
      updateBridge({ mathProgress: progress });
      return true;
    } catch {
      return false;
    }
  }

  function updateProgress(progress) {
    const completed = line.modules.filter((module) => progress[module.id]).length;
    const percent = line.modules.length ? Math.round((completed / line.modules.length) * 100) : 0;
    document.querySelector("#math-progress-text").textContent = `${completed} / ${line.modules.length} 已完成`;
    document.querySelector("#math-progress-fill").style.width = `${percent}%`;
  }

  function createModule(module, index, progress) {
    const article = document.createElement("article");
    article.className = "math-module";
    article.dataset.complete = String(Boolean(progress[module.id]));

    const header = document.createElement("div");
    header.className = "math-module-header";
    const order = document.createElement("span");
    order.className = "math-module-order";
    order.textContent = String(index + 1).padStart(2, "0");
    const heading = document.createElement("div");
    const id = document.createElement("small");
    id.textContent = module.id.toUpperCase();
    const title = document.createElement("h2");
    title.textContent = module.title;
    heading.append(id, title);
    const toggle = document.createElement("label");
    toggle.className = "math-complete-toggle";
    const checkbox = document.createElement("input");
    checkbox.type = "checkbox";
    checkbox.checked = Boolean(progress[module.id]);
    const toggleText = document.createElement("span");
    toggleText.textContent = checkbox.checked ? "已完成" : "标记完成";
    toggle.append(checkbox, toggleText);
    header.append(order, heading, toggle);

    const reason = document.createElement("p");
    reason.className = "math-module-why";
    reason.textContent = module.why;

    const grid = document.createElement("div");
    grid.className = "math-module-grid";
    const learn = document.createElement("section");
    learn.innerHTML = "<h3>需要学什么</h3>";
    const list = document.createElement("ul");
    module.learn.forEach((item) => {
      const li = document.createElement("li");
      li.textContent = item;
      list.append(li);
    });
    learn.append(list);
    const acceptance = document.createElement("section");
    acceptance.innerHTML = "<h3>过关标准</h3>";
    const pass = document.createElement("p");
    pass.textContent = module.pass;
    acceptance.append(pass);
    grid.append(learn, acceptance);

    const footer = document.createElement("div");
    footer.className = "math-module-footer";
    const related = document.createElement("span");
    related.textContent = `对应 Task5：${module.questions.map((id) => `问题 ${id}`).join("、")}`;
    footer.append(related);
    if (module.prerequisites?.length) {
      const prerequisites = document.createElement("span");
      prerequisites.textContent = `前置：${module.prerequisites.join("、")}`;
      footer.append(prerequisites);
    }

    checkbox.addEventListener("change", () => {
      if (checkbox.checked) progress[module.id] = true;
      else delete progress[module.id];
      toggleText.textContent = checkbox.checked ? "已完成" : "标记完成";
      article.dataset.complete = String(checkbox.checked);
      saveProgress(progress);
      updateProgress(progress);
    });

    article.append(header, reason, grid, footer);
    return article;
  }

  if (!line) {
    document.querySelector("main").innerHTML = '<section class="empty-state"><h1>未读取到数学任务</h1></section>';
    return;
  }

  document.title = `${line.id} · ${line.short} · Task5 OpenCV`;
  document.body.dataset.accent = line.accent;
  document.querySelector("#math-id").textContent = `${line.id.toUpperCase()} · ${line.short}`;
  document.querySelector("#math-title").textContent = line.title;
  document.querySelector("#math-description").textContent = line.description;

  const nav = document.querySelector("#math-line-nav");
  lines.forEach((item) => {
    const link = document.createElement("a");
    link.href = `math.html?task=${item.id}`;
    link.className = item.id === line.id ? "active" : "";
    link.innerHTML = `<small>${item.id.toUpperCase()}</small><strong>${item.short}</strong>`;
    nav.append(link);
  });

  const progress = readProgress();
  const fragment = document.createDocumentFragment();
  line.modules.forEach((module, index) => fragment.append(createModule(module, index, progress)));
  document.querySelector("#math-module-list").append(fragment);
  updateProgress(progress);
})();
