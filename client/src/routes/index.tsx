import { useEffect } from "react";
import Reader from "~/components/routes/index/reader";

const Index: React.FC = () => {
  // Remove the query and force hard reload without it for Discord OAuth
  useEffect(() => {
    const params = new URLSearchParams(window.location.search);
    if (params.get("success") === "true") {
      window.location.href = window.location.origin + window.location.pathname;
    }
  }, []);

  return <Reader />;
};

export default Index;
