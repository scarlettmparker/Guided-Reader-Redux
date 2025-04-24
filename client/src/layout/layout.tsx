import { JSX } from "react";
import Navbar from "~/components/navbar";

interface LayoutProps {
  children: JSX.Element;
}

const Layout: React.FC<LayoutProps> = (props) => {
  const { children } = props;

  return (
    <>
      <Navbar />
      {children}
    </>
  );
};

export default Layout;
