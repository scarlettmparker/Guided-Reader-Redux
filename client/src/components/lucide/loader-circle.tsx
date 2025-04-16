const LoaderCircle = () => (
  <svg
    xmlns="http://www.w3.org/2000/svg"
    width="24"
    height="24"
    viewBox="0 0 24 24"
    fill="none"
    stroke="currentColor"
    style={{ animation: "spin 1s linear infinite" }}
  >
    <path d="M21 12a9 9 0 1 1-6.219-8.56" />
    <style>
      {`
        @keyframes spin {
          0% {
            transform: rotate(0deg);
          }
          100% {
            transform: rotate(360deg);
          }
        }
      `}
    </style>
  </svg>
);

export default LoaderCircle;
