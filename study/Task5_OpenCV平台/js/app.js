(() => {
  "use strict";

  const questions = Array.isArray(window.OPENCV_QUESTIONS) ? window.OPENCV_QUESTIONS : [];
  const list = document.querySelector("#question-list");
  const search = document.querySelector("#question-search");
  const emptySearch = document.querySelector("#empty-search");
  const resultCount = document.querySelector("#search-result-count");
  const homeView = document.querySelector("#home-view");
  const questionView = document.querySelector("#question-view");
  const siteFooter = document.querySelector("#site-footer");
  const labels = {
    implement: "自己实现",
    "deep-read": "深入看懂",
    understand: "理解看懂",
  };
  const fullLabels = {
    implement: "深入原理 · 自己实现",
    "deep-read": "深入原理 · 看懂源码",
    understand: "理解原理 · 看懂源码",
  };

  let completed = {};
  let favorites = {};
  let notes = {};
  let recent = [];
  let saveTimer;
  const bridgePrefix = "TASK5_OFFLINE_STATE:";

  function readStorage(key, fallback) {
    try {
      const value = localStorage.getItem(key);
      return value ? JSON.parse(value) : fallback;
    } catch {
      return fallback;
    }
  }

  function writeStorage(key, value) {
    try {
      localStorage.setItem(key, JSON.stringify(value));
      return true;
    } catch {
      return false;
    }
  }

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
    const state = { ...readBridge(), ...patch };
    window.name = bridgePrefix + JSON.stringify(state);
  }

  function readLearningState() {
    completed = readStorage("task5-opencv-completed", {});
    favorites = readStorage("task5-opencv-favorites", {});
    notes = readStorage("task5-opencv-notes", {});
    recent = readStorage("task5-opencv-recent", []);
  }

  function createQuestionCard(question) {
    const article = document.createElement("a");
    article.className = `question-card ${question.difficulty}`;
    article.href = `#question=${question.id}`;

    const top = document.createElement("div");
    top.className = "card-top";
    const number = document.createElement("span");
    number.className = "question-number";
    number.textContent = String(question.id).padStart(3, "0");
    const difficulty = document.createElement("span");
    difficulty.className = `difficulty ${question.difficulty}`;
    difficulty.textContent = labels[question.difficulty];
    top.append(number, difficulty);

    const title = document.createElement("h3");
    title.textContent = question.title;

    const footer = document.createElement("div");
    footer.className = "card-status";
    const learningState = document.createElement("span");
    learningState.className = completed[question.id] ? "is-complete" : "";
    learningState.textContent = completed[question.id] ? "✓ 已完成" : "○ 未学习";
    const favoriteState = document.createElement("span");
    favoriteState.className = favorites[question.id] ? "is-favorite" : "";
    favoriteState.textContent = favorites[question.id] ? "★ 已收藏" : question.cpp ? "C++" : "暂无 C++";
    footer.append(learningState, favoriteState);
    article.append(top, title, footer);
    return article;
  }

  function searchableText(question) {
    const aliases = { 43: "NMS non-maximum suppression 非极大值抑制" };
    return `${question.id} ${question.title} ${question.readme} ${aliases[question.id] || ""}`.toLocaleLowerCase("zh-CN");
  }

  function filteredQuestions() {
    const query = search.value.trim().toLocaleLowerCase("zh-CN");
    return query ? questions.filter((question) => searchableText(question).includes(query)) : questions;
  }

  function renderQuestions(items = filteredQuestions()) {
    list.replaceChildren();
    const fragment = document.createDocumentFragment();
    items.forEach((question) => fragment.append(createQuestionCard(question)));
    list.append(fragment);
    resultCount.textContent = `${items.length} 题`;
    emptySearch.hidden = items.length !== 0;
  }

  function renderProgress() {
    const completeCount = questions.filter((question) => completed[question.id]).length;
    const percent = questions.length ? Math.round((completeCount / questions.length) * 100) : 0;
    document.querySelector("#completed-count").textContent = String(completeCount);
    document.querySelector("#total-count").textContent = String(questions.length);
    document.querySelector("#progress-fill").style.width = `${percent}%`;
    document.querySelector("#progress-label").textContent =
      completeCount === questions.length && questions.length ? "精选题已全部完成" : `已完成 ${percent}%`;
  }

  function renderRecent() {
    const section = document.querySelector("#recent-section");
    const recentList = document.querySelector("#recent-list");
    recentList.replaceChildren();
    section.hidden = true;
    const recentQuestions = recent
      .map((id) => questions.find((question) => question.id === Number(id)))
      .filter(Boolean)
      .slice(0, 5);
    if (!recentQuestions.length) return;
    recentQuestions.forEach((question) => {
      const link = document.createElement("a");
      link.href = `#question=${question.id}`;
      const number = document.createElement("span");
      number.textContent = String(question.id).padStart(3, "0");
      const title = document.createElement("strong");
      title.textContent = question.title;
      link.append(number, title);
      recentList.append(link);
    });
    section.hidden = false;
  }

  function renderMathProgress() {
    const lines = Array.isArray(window.TASK5_MATH_LINES) ? window.TASK5_MATH_LINES : [];
    const localProgress = readStorage("task5-math-progress", {});
    const bridgeProgress = readBridge().mathProgress || {};
    const progress = { ...localProgress, ...bridgeProgress };
    writeStorage("task5-math-progress", progress);
    updateBridge({ mathProgress: progress });
    lines.forEach((line) => {
      const target = document.querySelector(`#${line.id}-home-progress`);
      if (!target) return;
      const count = line.modules.filter((module) => progress[module.id]).length;
      target.textContent = `${count} / ${line.modules.length} 已完成`;
    });
  }

  function syncHome() {
    readLearningState();
    renderQuestions();
    renderProgress();
    renderRecent();
    renderMathProgress();
  }

  function recordRecent(questionId) {
    recent = [questionId, ...recent.filter((id) => Number(id) !== questionId)].slice(0, 5);
    writeStorage("task5-opencv-recent", recent);
  }

  function showSaveStatus(success = true) {
    const status = document.querySelector("#detail-save-status");
    status.textContent = success ? "已保存，首页进度已同步" : "浏览器未允许本地保存";
    clearTimeout(saveTimer);
    saveTimer = setTimeout(() => {
      status.textContent = "本地自动保存";
    }, 1800);
  }

  function setupDetailState(question) {
    const completedToggle = document.querySelector("#detail-completed-toggle");
    const favoriteToggle = document.querySelector("#detail-favorite-toggle");
    const note = document.querySelector("#detail-learning-note");
    completedToggle.checked = Boolean(completed[question.id]);
    note.value = notes[question.id] || "";

    const renderFavorite = () => {
      const active = Boolean(favorites[question.id]);
      favoriteToggle.classList.toggle("active", active);
      favoriteToggle.setAttribute("aria-pressed", String(active));
      favoriteToggle.textContent = active ? "★ 已收藏" : "☆ 收藏";
    };

    completedToggle.onchange = () => {
      if (completedToggle.checked) completed[question.id] = true;
      else delete completed[question.id];
      showSaveStatus(writeStorage("task5-opencv-completed", completed));
    };
    favoriteToggle.onclick = () => {
      if (favorites[question.id]) delete favorites[question.id];
      else favorites[question.id] = true;
      renderFavorite();
      showSaveStatus(writeStorage("task5-opencv-favorites", favorites));
    };
    note.oninput = () => {
      if (note.value) notes[question.id] = note.value;
      else delete notes[question.id];
      clearTimeout(saveTimer);
      document.querySelector("#detail-save-status").textContent = "正在保存……";
      saveTimer = setTimeout(() => showSaveStatus(writeStorage("task5-opencv-notes", notes)), 350);
    };
    renderFavorite();
  }

  function showQuestion(question) {
    homeView.hidden = true;
    questionView.hidden = false;
    siteFooter.hidden = true;
    document.title = `问题 ${question.id}：${question.title} · Task5 OpenCV`;
    document.querySelector("#detail-question-id").textContent = `QUESTION ${String(question.id).padStart(3, "0")}`;
    document.querySelector("#detail-question-title").textContent = question.title;
    const badge = document.querySelector("#detail-question-difficulty");
    badge.className = `difficulty ${question.difficulty}`;
    badge.textContent = fullLabels[question.difficulty];
    document.querySelector("#detail-readme-content").innerHTML = window.MarkdownLite.render(question.readme, question.image_map);
    document.querySelector("#detail-image-count").textContent = String(question.images.length);

    const codeLink = document.querySelector("#detail-code-link");
    codeLink.classList.toggle("disabled", !question.cpp);
    if (question.cpp) {
      codeLink.href = `code.html?id=${question.id}`;
      codeLink.textContent = "查看 C++ 代码";
    } else {
      codeLink.removeAttribute("href");
      codeLink.textContent = "原仓库暂无 C++ 答案";
    }

    const currentIndex = questions.indexOf(question);
    const previous = questions[currentIndex - 1];
    const next = questions[currentIndex + 1];
    const previousLink = document.querySelector("#detail-previous-question");
    const nextLink = document.querySelector("#detail-next-question");
    previousLink.classList.toggle("hidden", !previous);
    nextLink.classList.toggle("hidden", !next);
    if (previous) {
      previousLink.href = `#question=${previous.id}`;
      previousLink.textContent = `← 问题 ${previous.id}`;
    }
    if (next) {
      nextLink.href = `#question=${next.id}`;
      nextLink.textContent = `问题 ${next.id} →`;
    }

    recordRecent(question.id);
    setupDetailState(question);
    window.scrollTo(0, 0);
  }

  function showHome() {
    questionView.hidden = true;
    homeView.hidden = false;
    siteFooter.hidden = false;
    document.title = "Task5 · OpenCV 精选题";
    syncHome();
  }

  function importLegacyState(params, questionId) {
    let changed = false;
    if (params.get("done") === "1") {
      completed[questionId] = true;
      writeStorage("task5-opencv-completed", completed);
      changed = true;
    }
    if (params.get("favorite") === "1") {
      favorites[questionId] = true;
      writeStorage("task5-opencv-favorites", favorites);
      changed = true;
    }
    const legacyNote = params.get("note");
    if (legacyNote) {
      notes[questionId] = legacyNote;
      writeStorage("task5-opencv-notes", notes);
      changed = true;
    }
    if (changed) history.replaceState(null, "", `#question=${questionId}`);
  }

  function route() {
    readLearningState();
    const params = new URLSearchParams(window.location.hash.slice(1));
    const questionId = Number(params.get("question"));
    const question = questions.find((item) => item.id === questionId);
    if (question) {
      importLegacyState(params, questionId);
      showQuestion(question);
    } else {
      showHome();
    }
  }

  if (!questions.length) {
    list.innerHTML = '<p class="error-message">数据未加载。请先在 tools 目录运行 python3 generate.py。</p>';
    search.disabled = true;
    return;
  }

  search.addEventListener("input", () => renderQuestions());
  window.addEventListener("hashchange", route);
  window.addEventListener("pageshow", route);
  window.addEventListener("focus", () => {
    if (!homeView.hidden) syncHome();
  });
  document.addEventListener("visibilitychange", () => {
    if (document.visibilityState === "visible" && !homeView.hidden) syncHome();
  });
  window.addEventListener("storage", () => {
    if (!homeView.hidden) syncHome();
  });
  route();
})();
