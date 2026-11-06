(() => {
  "use strict";

  const questions = Array.isArray(window.OPENCV_QUESTIONS) ? window.OPENCV_QUESTIONS : [];
  const params = new URLSearchParams(window.location.search);
  const questionId = Number(params.get("id"));
  const question = questions.find((item) => item.id === questionId);
  const page = document.body.dataset.page;
  const labels = {
    implement: "深入原理 · 自己实现",
    "deep-read": "深入原理 · 看懂源码",
    understand: "理解原理 · 看懂源码",
  };

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

  // file:// 下不同 HTML 文件的 localStorage 可能彼此隔离。
  // 旧题目链接会携带当前题状态，转入 index.html 的同页详情视图。
  if (page === "question" && question) {
    const completed = readStorage("task5-opencv-completed", {});
    const favorites = readStorage("task5-opencv-favorites", {});
    const notes = readStorage("task5-opencv-notes", {});
    const transfer = new URLSearchParams({ question: String(question.id) });
    if (completed[question.id]) transfer.set("done", "1");
    if (favorites[question.id]) transfer.set("favorite", "1");
    if (notes[question.id]) transfer.set("note", notes[question.id]);
    window.location.replace(`index.html#${transfer.toString()}`);
    return;
  }

  function recordRecent() {
    const recent = readStorage("task5-opencv-recent", []);
    const next = [question.id, ...recent.filter((id) => Number(id) !== question.id)].slice(0, 5);
    writeStorage("task5-opencv-recent", next);
  }

  function setupLearningState() {
    const completed = readStorage("task5-opencv-completed", {});
    const favorites = readStorage("task5-opencv-favorites", {});
    const notes = readStorage("task5-opencv-notes", {});
    const completedToggle = document.querySelector("#completed-toggle");
    const favoriteToggle = document.querySelector("#favorite-toggle");
    const note = document.querySelector("#learning-note");
    const saveStatus = document.querySelector("#save-status");
    let saveTimer;

    completedToggle.checked = Boolean(completed[question.id]);
    note.value = notes[question.id] || "";

    const renderFavorite = () => {
      const active = Boolean(favorites[question.id]);
      favoriteToggle.classList.toggle("active", active);
      favoriteToggle.setAttribute("aria-pressed", String(active));
      favoriteToggle.textContent = active ? "★ 已收藏" : "☆ 收藏";
    };
    const showSaved = (success = true) => {
      saveStatus.textContent = success ? "已保存到本地" : "浏览器未允许本地保存";
      clearTimeout(saveTimer);
      saveTimer = setTimeout(() => {
        saveStatus.textContent = "本地自动保存";
      }, 1800);
    };

    completedToggle.addEventListener("change", () => {
      if (completedToggle.checked) completed[question.id] = true;
      else delete completed[question.id];
      showSaved(writeStorage("task5-opencv-completed", completed));
    });
    favoriteToggle.addEventListener("click", () => {
      if (favorites[question.id]) delete favorites[question.id];
      else favorites[question.id] = true;
      renderFavorite();
      showSaved(writeStorage("task5-opencv-favorites", favorites));
    });
    note.addEventListener("input", () => {
      if (note.value) notes[question.id] = note.value;
      else delete notes[question.id];
      clearTimeout(saveTimer);
      saveStatus.textContent = "正在保存……";
      saveTimer = setTimeout(() => showSaved(writeStorage("task5-opencv-notes", notes)), 350);
    });
    renderFavorite();
  }

  function showMissing() {
    document.title = "题目不存在 · Task5 OpenCV";
    const main = document.querySelector("main");
    main.innerHTML = `<section class="empty-state"><span>404</span><h1>未找到这道精选题</h1><p>请返回首页从题目列表进入。</p><a class="primary-button" href="index.html">返回题库</a></section>`;
  }

  function setSharedHeader() {
    document.title = `问题 ${question.id}：${question.title} · Task5 OpenCV`;
    document.querySelector("#question-id").textContent = `QUESTION ${String(question.id).padStart(3, "0")}`;
    document.querySelector("#question-title").textContent = question.title;
    const badge = document.querySelector("#question-difficulty");
    badge.className = `difficulty ${question.difficulty}`;
    badge.textContent = labels[question.difficulty];
  }

  function renderQuestion() {
    setSharedHeader();
    recordRecent();
    document.querySelector("#readme-content").innerHTML = window.MarkdownLite.render(
      question.readme,
      question.image_map,
    );
    const codeLink = document.querySelector("#code-link");
    if (question.cpp) {
      codeLink.href = `code.html?id=${question.id}`;
      codeLink.textContent = "查看 C++ 代码";
    } else {
      codeLink.removeAttribute("href");
      codeLink.classList.add("disabled");
      codeLink.textContent = "原仓库暂无 C++ 答案";
    }
    document.querySelector("#image-count").textContent = String(question.images.length);
    setupLearningState();

    const currentIndex = questions.indexOf(question);
    const previous = questions[currentIndex - 1];
    const next = questions[currentIndex + 1];
    const previousLink = document.querySelector("#previous-question");
    const nextLink = document.querySelector("#next-question");
    if (previous) {
      previousLink.href = `question.html?id=${previous.id}`;
      previousLink.textContent = `← 问题 ${previous.id}`;
    } else previousLink.classList.add("hidden");
    if (next) {
      nextLink.href = `question.html?id=${next.id}`;
      nextLink.textContent = `问题 ${next.id} →`;
    } else nextLink.classList.add("hidden");
  }

  function copyCode(button) {
    const finish = () => {
      button.textContent = "已复制";
      setTimeout(() => (button.textContent = "复制代码"), 1500);
    };
    if (navigator.clipboard?.writeText) {
      navigator.clipboard.writeText(question.cpp).then(finish).catch(() => fallbackCopy(finish));
    } else fallbackCopy(finish);
  }

  function fallbackCopy(finish) {
    const area = document.createElement("textarea");
    area.value = question.cpp;
    area.style.position = "fixed";
    area.style.opacity = "0";
    document.body.append(area);
    area.select();
    document.execCommand("copy");
    area.remove();
    finish();
  }

  function renderCode() {
    setSharedHeader();
    const backLink = document.querySelector("#code-back-link");
    if (backLink) backLink.href = `index.html#question=${question.id}`;
    if (!question.cpp) {
      document.querySelector("#code-panel").innerHTML = '<div class="empty-code">原仓库没有这道题的 C++ 答案。</div>';
      return;
    }
    document.querySelector("#code-content").innerHTML = window.MarkdownLite.highlightCpp(question.cpp);
    document.querySelector("#code-lines").textContent = `${question.cpp.split("\n").length} 行`;
    document.querySelector("#copy-code").addEventListener("click", (event) => copyCode(event.currentTarget));
  }

  if (!question) {
    showMissing();
  } else if (page === "question") {
    renderQuestion();
  } else if (page === "code") {
    renderCode();
  }
})();
