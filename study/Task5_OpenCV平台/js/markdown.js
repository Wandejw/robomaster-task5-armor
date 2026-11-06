(() => {
  "use strict";

  const escapeHtml = (value) =>
    String(value)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;")
      .replaceAll("'", "&#039;");

  function formatMath(value, displayMode = false) {
    const source = String(value).trim();
    if (window.katex?.renderToString) {
      try {
        return window.katex.renderToString(source, {
          displayMode,
          throwOnError: true,
          strict: "ignore",
          trust: false,
          output: "htmlAndMathml",
        });
      } catch (error) {
        console.warn("数学公式解析失败，已显示原始公式：", source, error);
      }
    }
    return `<code class="math-fallback">${escapeHtml(source)}</code>`;
  }

  function renderInline(source, imageMap = {}) {
    const tokens = [];
    const stash = (html) => {
      const key = `@@MDTOKEN${tokens.length}@@`;
      tokens.push(html);
      return key;
    };

    let text = String(source);
    text = text.replace(/<img\b[^>]*?\bsrc=["']([^"']+)["'][^>]*>/gi, (_, src) => {
      const resolved = imageMap[src] || src;
      return stash(`<img src="${escapeHtml(resolved)}" alt="题目示例图">`);
    });
    text = text.replace(/!\[([^\]]*)\]\((?:<)?([^\s)>]+)(?:>?)\)/g, (_, alt, src) => {
      const resolved = imageMap[src] || src;
      return stash(`<img src="${escapeHtml(resolved)}" alt="${escapeHtml(alt || "题目示例图")}">`);
    });
    text = text.replace(/`([^`]+)`/g, (_, code) => stash(`<code>${escapeHtml(code)}</code>`));
    text = text.replace(/\$([^$\n]+)\$/g, (_, math) => stash(`<span class="math-inline">${formatMath(math, false)}</span>`));

    text = escapeHtml(text)
      .replace(/\*\*([^*]+)\*\*/g, "<strong>$1</strong>")
      .replace(/__([^_]+)__/g, "<strong>$1</strong>")
      .replace(/\[([^\]]+)\]\(([^)]+)\)/g, '<span class="source-reference">$1</span>');

    tokens.forEach((html, index) => {
      text = text.replaceAll(`@@MDTOKEN${index}@@`, html);
    });
    return text;
  }

  const isTableSeparator = (line) =>
    /^\s*\|?\s*:?-{1,}:?\s*(?:\|\s*:?-{1,}:?\s*)+\|?\s*$/.test(line);

  function tableCells(line) {
    let value = line.trim();
    if (value.startsWith("|")) value = value.slice(1);
    if (value.endsWith("|")) value = value.slice(0, -1);
    return value.split("|").map((cell) => cell.trim());
  }

  function render(markdown, imageMap = {}) {
    const lines = String(markdown).replaceAll("\r\n", "\n").split("\n");
    const output = [];
    let index = 0;

    while (index < lines.length) {
      const line = lines[index];
      if (!line.trim()) {
        index += 1;
        continue;
      }

      if (line.trim().startsWith("```")) {
        const language = line.trim().slice(3).trim();
        const code = [];
        index += 1;
        while (index < lines.length && !lines[index].trim().startsWith("```")) {
          code.push(lines[index]);
          index += 1;
        }
        index += 1;
        output.push(`<pre><code data-language="${escapeHtml(language)}">${escapeHtml(code.join("\n"))}</code></pre>`);
        continue;
      }

      if (line.trim() === "$$") {
        const math = [];
        index += 1;
        while (index < lines.length && lines[index].trim() !== "$$") {
          math.push(lines[index]);
          index += 1;
        }
        index += 1;
        output.push(`<div class="math-block">${formatMath(math.join("\n"), true)}</div>`);
        continue;
      }

      const heading = line.match(/^(#{1,6})\s+(.+)$/);
      if (heading) {
        const level = heading[1].length;
        output.push(`<h${level}>${renderInline(heading[2], imageMap)}</h${level}>`);
        index += 1;
        continue;
      }

      if (line.includes("|") && index + 1 < lines.length && isTableSeparator(lines[index + 1])) {
        const header = tableCells(line);
        index += 2;
        const rows = [];
        while (index < lines.length && lines[index].includes("|") && lines[index].trim()) {
          rows.push(tableCells(lines[index]));
          index += 1;
        }
        output.push(
          `<div class="table-scroll"><table><thead><tr>${header.map((cell) => `<th>${renderInline(cell, imageMap)}</th>`).join("")}</tr></thead><tbody>${rows
            .map((row) => `<tr>${row.map((cell) => `<td>${renderInline(cell, imageMap)}</td>`).join("")}</tr>`)
            .join("")}</tbody></table></div>`,
        );
        continue;
      }

      if (/^\s*>/.test(line)) {
        const quote = [];
        while (index < lines.length && /^\s*>/.test(lines[index])) {
          quote.push(lines[index].replace(/^\s*>\s?/, ""));
          index += 1;
        }
        output.push(`<blockquote>${quote.map((item) => renderInline(item, imageMap)).join("<br>")}</blockquote>`);
        continue;
      }

      const unordered = line.match(/^\s*[-*+]\s+(.+)$/);
      const ordered = line.match(/^\s*\d+[.)]\s+(.+)$/);
      if (unordered || ordered) {
        const tag = ordered ? "ol" : "ul";
        const items = [];
        const pattern = ordered ? /^\s*\d+[.)]\s+(.+)$/ : /^\s*[-*+]\s+(.+)$/;
        while (index < lines.length) {
          const match = lines[index].match(pattern);
          if (!match) break;
          items.push(`<li>${renderInline(match[1], imageMap)}</li>`);
          index += 1;
        }
        output.push(`<${tag}>${items.join("")}</${tag}>`);
        continue;
      }

      const paragraph = [line.trim()];
      index += 1;
      while (
        index < lines.length &&
        lines[index].trim() &&
        !/^(#{1,6})\s+/.test(lines[index]) &&
        lines[index].trim() !== "$$" &&
        !lines[index].trim().startsWith("```") &&
        !/^\s*>/.test(lines[index]) &&
        !/^\s*(?:[-*+]|\d+[.)])\s+/.test(lines[index]) &&
        !(lines[index].includes("|") && index + 1 < lines.length && isTableSeparator(lines[index + 1]))
      ) {
        paragraph.push(lines[index].trim());
        index += 1;
      }
      output.push(`<p>${renderInline(paragraph.join(" "), imageMap)}</p>`);
    }

    return output.join("\n");
  }

  function highlightCpp(code) {
    const pattern = /(\/\/[^\n]*|\/\*[\s\S]*?\*\/)|("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')|(^\s*#[^\n]*)|\b(alignas|alignof|and|auto|bool|break|case|catch|char|class|const|constexpr|continue|default|delete|do|double|else|enum|explicit|false|float|for|friend|if|inline|int|long|namespace|new|nullptr|operator|private|protected|public|return|short|signed|sizeof|static|struct|switch|template|this|throw|true|try|typedef|typename|union|unsigned|using|virtual|void|volatile|while)\b|\b(\d+(?:\.\d+)?)\b/gm;
    let result = "";
    let cursor = 0;
    for (const match of code.matchAll(pattern)) {
      result += escapeHtml(code.slice(cursor, match.index));
      const className = match[1]
        ? "syntax-comment"
        : match[2]
          ? "syntax-string"
          : match[3]
            ? "syntax-preprocessor"
            : match[4]
              ? "syntax-keyword"
              : "syntax-number";
      result += `<span class="${className}">${escapeHtml(match[0])}</span>`;
      cursor = match.index + match[0].length;
    }
    return result + escapeHtml(code.slice(cursor));
  }

  window.MarkdownLite = { render, highlightCpp };
})();
