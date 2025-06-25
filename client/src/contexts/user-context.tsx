import { createContext, useContext, useState, ReactNode } from "react";

interface UserData {
  username: string;
  [key: string]: any;
}

interface UserContextType {
  user: UserData | null;
  setUser: (user: UserData | null) => void;
}

interface UserProviderProps {
  children: ReactNode;
  initialUser?: UserData | null;
}

const UserContext = createContext<UserContextType | undefined>(undefined);

export function UserProvider({
  children,
  initialUser = null,
}: UserProviderProps) {
  const [user, setUser] = useState<UserData | null>(initialUser);

  return (
    <UserContext.Provider value={{ user, setUser }}>
      {children}
    </UserContext.Provider>
  );
}

export function useUser() {
  const ctx = useContext(UserContext);
  if (!ctx) throw new Error("useUser must be used within a UserProvider");
  return ctx;
}
