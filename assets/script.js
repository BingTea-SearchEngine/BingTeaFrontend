document.addEventListener("DOMContentLoaded", function () {
  const form = document.querySelector("form");
  const input = document.querySelector("input[name='query']");

  form.addEventListener("submit", function (event) {
      event.preventDefault(); // Prevent page reload

      const query = input.value.trim();
      if (!query) return;

      console.log("User searched for:", query);

      // Example: Send the query to a server or device for processing
      fetch("http://localhost:8001/search", {
          method: "POST",
          headers: {
              "Content-Type": "application/json",
          },
          body: JSON.stringify({ query }),
      })
      .then(response => response.json())
      .then(data => console.log("Search Results:", data))
      .catch(error => console.error("Error:", error));
  });
});
