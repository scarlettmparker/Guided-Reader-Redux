import { JSX } from "react";
import { UserProvider } from "~/contexts/user-context";
import Navbar from "~/components/navbar";

interface LayoutProps {
  children: JSX.Element;
  user?: any;
}

const Layout: React.FC<LayoutProps> = (props) => {
  const { children, user } = props;

  return (
    <UserProvider initialUser={user}>
      <Navbar />
      {children}
    </UserProvider>
  );
};

export default Layout;
