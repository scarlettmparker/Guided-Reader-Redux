import React from "react";

interface ErrorStateProps {
  children: React.ReactNode;
}

const ErrorState: React.FC<ErrorStateProps> = ({ children }) => {
  return <span>{children}</span>;
};

export default ErrorState;
